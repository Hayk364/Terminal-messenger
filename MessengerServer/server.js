// Import required libraries
import express from 'express'          // For creating HTTP server
import crypto  from 'crypto'           // For encryption and decryption
import dotenv from 'dotenv'            // For loading environment variables from .env file
import { MongoClient } from 'mongodb'  // For MongoDB database interaction

const app = express()

// Middleware to parse incoming JSON requests
app.use(express.json());

dotenv.config() // Load environment variables from .env file

const PORT = 4040

// MongoDB connection URI (local instance)
const uri = "mongodb://localhost:27017";

// Database name
const dbName = "Messenger";

// AES-256 encryption key loaded from environment variable (hex format)
const GLOBALKEY = Buffer.from(process.env.GLOBAL_KEY, 'hex');

/**
 * Encrypts a plain text string using AES-256-CBC.
 * Generates a random Initialization Vector (IV) each time.
 * Returns an object containing the encrypted content and IV as hex strings.
 */
function encrypt(text){
	const iv = crypto.randomBytes(16)  // Generate 16 bytes random IV
	const cipher = crypto.createCipheriv('aes-256-cbc', GLOBALKEY, iv)
	let encrypted = cipher.update(text, 'utf-8', 'hex')
	encrypted += cipher.final('hex')
	return {
		content: encrypted,       // Encrypted text as hex string
		iv: iv.toString('hex')    // Initialization Vector as hex string
	}
}

/**
 * Decrypts data encrypted with the above encrypt() function.
 * Takes an object with 'content' and 'iv' (both hex strings).
 * Returns the original decrypted plain text.
 */
function decrypt(encrypted){
	const iv = Buffer.from(encrypted.iv, 'hex')  // Convert IV back to Buffer
	const decipher = crypto.createDecipheriv('aes-256-cbc', GLOBALKEY, iv);
	let decrypted = decipher.update(encrypted.content, 'hex', 'utf-8')
	decrypted += decipher.final('utf-8')
	return decrypted
}

let db;

// Connect to MongoDB database
const client = new MongoClient(uri);
try {
    await client.connect();
    db = client.db(dbName); 
    console.log("Database connected successfully");
} catch (error) {
    console.error("Client connection error:", error);
}

// User registration endpoint
app.post('/register', async (req, res) => {
	try {
		const username = req.body.username
		const password = req.body.password

		// Encrypt the user's password before saving
		const hashPassword = encrypt(password)

		// Get last user ID to increment for new user
		const lastElement = await db.collection("users").find().sort({ _id: -1 }).limit(1).toArray()
		const nextId = (lastElement[0]?.id ?? 0) + 1;

		// Insert new user document with encrypted password and unique ID
		const result = await db.collection("users").insertOne({
			username: username,
			password: {
				encryptedData: hashPassword.content,
				iv: hashPassword.iv
			},
			id: nextId
		})

		if (result) {
			res.json({ success: true })
		} else {
			res.json({ success: false })
		}

	} catch (error) {
		console.log(error)
	}
})

// User login endpoint
app.post('/login', async (req, res) => {
	try {
		const username = req.body.username
		const password = req.body.password

		// Find user by username
		const result = await db.collection("users").findOne({ username: username })

		// If user exists and decrypted password matches input, login success
		if (result && decrypt({
			content: result.password.encryptedData,
			iv: result.password.iv
		}) == password) {
			res.json({ success: true })
		} else {
			res.json({ success: false })
		}
	} catch (error) {
		console.log(error)
	}
})

// Send message endpoint
app.post('/send-message', async (req, res) => {
	try {
		const username = req.body.username
		const friendname = req.body.friendname
		const message = req.body.message

		// Encrypt the message content before saving
		const hashMessage = encrypt(message)

		// Save the message to the chats collection
		const result = await db.collection("chats").insertOne({
			sendername: username,
			gettername: friendname,
			message: {
				encryptedData: hashMessage.content,
				iv: hashMessage.iv
			}
		})

		if (result) {
			res.json({ success: true })
		} else {
			res.json({ success: false })
		}
	} catch (error) {
		console.log(error)
	}
})

// Get chat messages between two users endpoint
app.post('/get-chat', async (req, res) => {
	try {
		const username = req.body.username
		const friendname = req.body.friendname

		// Query chats collection for messages between the two users (both directions)
		const result = await db.collection("chats").find({
			$or: [
				{ sendername: username, gettername: friendname },
				{ sendername: friendname, gettername: username }
			]
		}).toArray()

		let array = []

		// Decrypt each message before sending to client
		result.forEach(el => {
			array.push({
				sendername: el.sendername,
				gettername: el.gettername,
				message: decrypt({
					content: el.message.encryptedData,
					iv: el.message.iv
				})
			})
		})

		// Respond with the decrypted chat messages array
		res.json(array)

	} catch (error) {
		console.log(error)
	}
})

// Get list of users except the requesting user
app.post('/get-users', async (req, res) => {
	try {
		const username = req.body.username

		// Retrieve all users from DB
		const result = await db.collection("users").find().toArray()

		let array = []

		// Filter out the requesting user and format response
		result.forEach(el => {
			if (username !== el.username) {
				array.push({ id: parseInt(el.id), username: el.username })
			}
		})

		// Send back the list of other users
		res.json(array)
	} catch (error) {
		console.log(error)
	}
})

// Start the server
app.listen(PORT, () => {
	console.log(`Server running at http://127.0.0.1:${PORT}`)
})
