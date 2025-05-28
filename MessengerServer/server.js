import express from 'express'
import crypto  from 'crypto'
import dotenv from 'dotenv'
import { MongoClient } from 'mongodb'
const app = express()

app.use(express.json());

dotenv.config()

const PORT = 4040
const uri = "mongodb://localhost:27017";
const dbName = "Messenger";


const GLOBALKEY = Buffer.from(process.env.GLOBAL_KEY, 'hex');


function encrypt(text){
	const iv = crypto.randomBytes(16)
	const cipher = crypto.createCipheriv('aes-256-cbc',GLOBALKEY,iv)
	let encrypted = cipher.update(text,'utf-8','hex')
	encrypted+= cipher.final('hex')
	return {
		content:encrypted,
		iv:iv.toString('hex')
	}
}

function decrypt(encrypted){
	const iv = Buffer.from(encrypted.iv,'hex')
	const decipher = crypto.createDecipheriv('aes-256-cbc',GLOBALKEY,iv);
	let decrypted = decipher.update(encrypted.content,'hex','utf-8')
	decrypted += decipher.final('utf-8')
	return decrypted
}

let db;

const client = new MongoClient(uri);
try {
    await client.connect();
    db = client.db(dbName); 
    console.log("Database connected successfully");
} catch (error) {
    console.error("Client connection error:", error);
}



app.post('/register',async (req,res) => {
	try{
		const username = req.body.username
		const password = req.body.password

		const hashPassword = encrypt(password)

		const lastElement = await db.collection("users").find().sort({ _id: -1 }).limit(1).toArray()
		const nextId = (lastElement[0]?.id ?? 0) + 1;

		const result = await db.collection("users").insertOne({username:username,password:{encryptedData:hashPassword.content,iv:hashPassword.iv},id:nextId})
		if(result){
			res.json({success:true})
		}else{
			res.json({success:false})
		}

	}catch(error){
		console.log(error)
	}
})

app.post('/login',async (req,res) => {
	try{
		const username = req.body.username
		const password = req.body.password

		const result = await db.collection("users").findOne({username:username})
		if(result && decrypt({content:result.password.encryptedData,iv:result.password.iv}) == password){
			res.json({success:true})
		}else{
			res.json({success:false})
		}
	}catch(error){
		console.log(error)
	}
})

app.post('/send-message',async (req,res) => {
	try{
		const username = req.body.username
		const friendname = req.body.friendname
		const message = req.body.message

		const hashMessage = encrypt(message)
		const result = await db.collection("chats").insertOne({sendername:username,gettername:friendname,message:{encryptedData:hashMessage.content,iv:hashMessage.iv}})

		if(result){
			res.json({success:true})
		}else{
			res.json({success:false})
		}
	}catch(error){
		console.log(error)
	}
})

app.post('/get-chat',async (req,res) => {
	try{
		const username = req.body.username
		const friendname = req.body.friendname

		const result = await db.collection("chats").find({
			$or:[
				{sendername:username,gettername:friendname},
				{sendername:friendname,gettername:username}
			]
		}).toArray()
		let array = [];
		result.forEach(el => {
      		array.push({
        		sendername: el.sendername,
        		gettername: el.gettername,
        		message: decrypt({
          			content: el.message.encryptedData,
          			iv: el.message.iv
        		})
      		});
    	});
    	res.json(array);

	}catch(error){
		console.log(error)
	}
})

app.post('/get-users',async (req,res) => {
	try{
		const username = req.body.username

		const result = await db.collection("users").find().toArray()

		let array = []

		result.forEach(el => {
			if(username == el.username){}else{
				array.push({id:parseInt(el.id),username:el.username})
			}
		})

		res.json(array)
	}catch(error){
		console.log(error)
	}
})


app.listen(PORT,() => {
	console.log(`http://127.0.0.1:${PORT}`)
})














