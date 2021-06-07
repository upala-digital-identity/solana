/* eslint-disable @typescript-eslint/no-unsafe-assignment */
/* eslint-disable @typescript-eslint/no-unsafe-member-access */

import {
  Keypair,
  Connection,
  PublicKey,
  LAMPORTS_PER_SOL,
  SystemProgram,
  SYSVAR_RENT_PUBKEY,
  TransactionInstruction,
  Transaction,
  sendAndConfirmTransaction,
} from '@solana/web3.js';
import fs from 'mz/fs';
import path from 'path';

import { 
  TOKEN_PROGRAM_ID,
  Token,
  AccountInfo
} from '@solana/spl-token';

import {
  readAccountFromFile,
} from './utils';

enum UpalaInstution
{
  UI_CreatePool,   // 0
  UI_EmptyPool,    // 1
  UI_RemovePool,   // 2
  UI_AddUser,      // 3
  UI_RemoveUser,   // 4
  UI_SetScore,     // 5
  UI_CleanStorage  // 6
};

/**
 * Connection to the network
 */
let connection: Connection;

const WALLET_PATH = './wallet';
const PROGRAM_KEY_FILE = './dist/program/helloworld-keypair.json';

const UPALA = path.join(WALLET_PATH, 'upala');
const MANAGER_PATH = path.join(UPALA, 'manager.json');
export let   UPALA_PROGRAM_ID:PublicKey;//new PublicKey('4KM6w8LY2gKhxp7943N1AowyodHay16bhVaQLm7gtJ64');

const MINTER = path.join(WALLET_PATH, 'tokens/shittoken');
const TOKEN_KEYPAIR_PATH = path.join(MINTER, 'shittoken.json');
const TOKEN_PAYER_PATH = path.join(MINTER, 'payer.json');
export let   TOKEN_ID:PublicKey;

export const USER_1_KEYPAIR_PATH = path.join(UPALA, 'account_1.json');

/**
 * Establish a connection to the cluster
 */
export async function establishConnection(): Promise<void> {
  const testnetUrl = 'https://api.testnet.solana.com';
  connection = new Connection(testnetUrl, 'confirmed');
  const version = await connection.getVersion();
  console.log('Connection to cluster established:', testnetUrl, version);
}

export async function loadProgramId(): Promise<void> 
{
  UPALA_PROGRAM_ID = (await readAccountFromFile(PROGRAM_KEY_FILE)).publicKey
}

export async function loadTokenId(): Promise<void> 
{
  TOKEN_ID = new PublicKey("5GczvdfV9zanGsDQjmQKMxKMEVrRVBRgfdSXP7zpRCYf");
}


export async function loadManager(): Promise<Keypair> 
{
  return await readAccountFromFile(MANAGER_PATH);
}

export async function createGroupPoolAddress(keys: Array<PublicKey>, program_id: PublicKey): Promise<PublicKey> 
{
  return await findAssociatedTokenAddress(keys, program_id);
}

async function findAssociatedTokenAddress(
  ids: Array<PublicKey>,
  program_id: PublicKey
): Promise<PublicKey>
{
  return (await PublicKey.findProgramAddress(
      ids.map((item:PublicKey) => item.toBuffer()),
      program_id
  ))[0];
}

export async function printPubkey(key:string): Promise<Uint8Array> 
{
  let pk:PublicKey = new PublicKey(key);
  return Uint8Array.from(pk.toBuffer());
}

export async function create(): Promise<PublicKey>
{
  const manager:Keypair = await loadManager();
  const pool_at_account:PublicKey = await createGroupPoolAddress([manager.publicKey, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Associated token account:', pool_at_account.toBase58());

  const pools_manager_account:PublicKey = await createGroupPoolAddress([TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Upala manager account:', pools_manager_account.toBase58());

  const data_instruction = Buffer.alloc(1);
  data_instruction.writeUInt8(UpalaInstution.UI_CreatePool, 0);
  console.log("Data instruction of UpalaInstution.UI_CreatePool (hex):", data_instruction.toString('hex'));
  
  const instruction = new TransactionInstruction(
    {
    keys: [
        {pubkey: manager.publicKey,         isSigner: true, isWritable: true},   // 0
        {pubkey: pool_at_account,           isSigner: false, isWritable: true},  // 1
        {pubkey: pools_manager_account,     isSigner: false, isWritable: true},  // 2
        {pubkey: TOKEN_ID,                  isSigner: false, isWritable: false}, // 3
        {pubkey: UPALA_PROGRAM_ID,          isSigner: false, isWritable: false}, // 4 
        {pubkey: SystemProgram.programId,   isSigner: false, isWritable: false}, // 5
        {pubkey: SYSVAR_RENT_PUBKEY,        isSigner: false, isWritable: false}, // 6
        {pubkey: TOKEN_PROGRAM_ID,          isSigner: false, isWritable: false}, // 7
      ],
    programId: UPALA_PROGRAM_ID,
    data: data_instruction,
  });
  
  let tx = new Transaction().add(instruction);
	tx.recentBlockhash	= (await connection.getRecentBlockhash()).blockhash;
	tx.feePayer			    = manager.publicKey;
	tx.sign(manager);

	const signers = [manager];
	const resultTxSimul = await connection.simulateTransaction(tx, signers);
	const txResponse = resultTxSimul.value;
	
  console.error("Transaction error:", txResponse.err);
  if (txResponse.err == null)
	{
		console.log('Transaction Signature (create associated token account)', 
      await sendAndConfirmTransaction(
        connection,
        tx,
        signers
      ));
	}
	console.log(txResponse.logs);

  return pool_at_account;
}

export async function cleanStorage(): Promise<PublicKey>
{
  const manager:Keypair = await loadManager();
  console.log('Group manager account:', manager.publicKey.toBase58());

  const associated_account_address:PublicKey = await createGroupPoolAddress([manager.publicKey, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Associated token account:', associated_account_address.toBase58());

  const upala_manager_address:PublicKey = await createGroupPoolAddress([TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Upala manager account:', associated_account_address.toBase58());

  const data_instruction = Buffer.alloc(1);
  data_instruction.writeUInt8(UpalaInstution.UI_CleanStorage, 0);
  console.log("Data instruction of UpalaInstution.UI_CleanStorage (hex):", data_instruction.toString('hex'));
  
  const instruction = new TransactionInstruction(
    {
    keys: [
        {pubkey: manager.publicKey,         isSigner: true, isWritable: true},   // 0
        {pubkey: associated_account_address,isSigner: false, isWritable: true},  // 1
        {pubkey: upala_manager_address,     isSigner: false, isWritable: true},  // 2
        {pubkey: TOKEN_ID,                  isSigner: false, isWritable: false}, // 3
        {pubkey: UPALA_PROGRAM_ID,          isSigner: false, isWritable: false}, // 4 
        {pubkey: SystemProgram.programId,   isSigner: false, isWritable: false}, // 5
        {pubkey: SYSVAR_RENT_PUBKEY,        isSigner: false, isWritable: false}, // 6
        {pubkey: TOKEN_PROGRAM_ID,          isSigner: false, isWritable: false}, // 7
      ],
    programId: UPALA_PROGRAM_ID,
    data: data_instruction,
  });
  
  let tx = new Transaction().add(instruction);
	tx.recentBlockhash	= (await connection.getRecentBlockhash()).blockhash;
	tx.feePayer			    = manager.publicKey;
	tx.sign(manager);

	const signers = [manager];
	const resultTxSimul = await connection.simulateTransaction(tx, signers);
	const txResponse = resultTxSimul.value;
	
  console.error("Transaction error:", txResponse.err);
  if (txResponse.err == null)
	{
		console.log('Transaction Signature (create associated token account)', 
      await sendAndConfirmTransaction(
        connection,
        tx,
        signers
      ));
	}
	console.log(txResponse.logs);

  return associated_account_address;
}

export async function addUser(group_id: PublicKey, user_account: PublicKey, score: Number): Promise<PublicKey>
{
  console.log('User account:', user_account.toBase58(), 'Score:', score);

  const manager:Keypair = await loadManager();
  console.log('Group manager account:', manager.publicKey.toBase58());

  const pool_at_account:PublicKey = group_id;
  console.log('Associated token account:', pool_at_account.toBase58());

  const user_at_account:PublicKey = await createGroupPoolAddress([user_account, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Associated token account:', user_at_account.toBase58());

  const pools_manager_account:PublicKey = await createGroupPoolAddress([TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Upala manager account:', pools_manager_account.toBase58());

  const data_instruction = Buffer.alloc(1);
  const buffer_cmd = Buffer.alloc(1);
  buffer_cmd.writeUInt8(UpalaInstution.UI_AddUser, 0);
  const buffer_count = Buffer.alloc(1);
  buffer_count.writeUInt8(1, 0);
  const buffer_score = Buffer.alloc(8);
  buffer_score.writeBigUInt64LE(BigInt(score), 0);
  
  const data = Buffer.concat(
    [
      buffer_cmd,
      group_id.toBuffer(),
      buffer_count,
      user_account.toBuffer(),
      buffer_score
    ]
  );
  console.log("Data instruction of UpalaInstution.UI_AddUser (hex):", data.toString('hex'));
  
  const instruction = new TransactionInstruction(
    {
    keys: [
        {pubkey: manager.publicKey,         isSigner: true, isWritable: true},   // 0
        {pubkey: pool_at_account,           isSigner: false, isWritable: true},  // 1
        {pubkey: pools_manager_account,     isSigner: false, isWritable: true},  // 2
        {pubkey: TOKEN_ID,                  isSigner: false, isWritable: false}, // 3
        {pubkey: UPALA_PROGRAM_ID,          isSigner: false, isWritable: false}, // 4 
        {pubkey: SystemProgram.programId,   isSigner: false, isWritable: false}, // 5
        {pubkey: SYSVAR_RENT_PUBKEY,        isSigner: false, isWritable: false}, // 6
        {pubkey: TOKEN_PROGRAM_ID,          isSigner: false, isWritable: false}, // 7
        {pubkey: user_account,              isSigner: false, isWritable: false}, // 8
        {pubkey: user_at_account,           isSigner: false, isWritable: true}, // 8
      ],
    programId: UPALA_PROGRAM_ID,
    data: data,
  });
  
  let tx = new Transaction().add(instruction);
	tx.recentBlockhash	= (await connection.getRecentBlockhash()).blockhash;
	tx.feePayer			    = manager.publicKey;
	tx.sign(manager);

	const signers = [manager];
	const resultTxSimul = await connection.simulateTransaction(tx, signers);
	const txResponse = resultTxSimul.value;
	
  console.error("Transaction error:", txResponse.err);
  if (txResponse.err == null)
	{
		console.log('Transaction Signature (create associated token account)', 
      await sendAndConfirmTransaction(
        connection,
        tx,
        signers
      ));
	}
	console.log(txResponse.logs);

  return pool_at_account;
}

export async function empty(user_account: Keypair): Promise<PublicKey>
{
  const manager:Keypair = await loadManager();
  console.log('Group manager account:', manager.publicKey.toBase58());

  const pool_at_account:PublicKey = await createGroupPoolAddress([manager.publicKey, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Associated token account:', pool_at_account.toBase58());

  const user_at_account:PublicKey = await createGroupPoolAddress([user_account.publicKey, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Dst token account:', user_at_account.toBase58());

  const pools_manager_account:PublicKey = await createGroupPoolAddress([TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Upala manager account:', pools_manager_account.toBase58());

  const buffer_cmd = Buffer.alloc(1);
  buffer_cmd.writeUInt8(UpalaInstution.UI_EmptyPool, 0);

  console.log("Data instruction of UpalaInstution.UI_EmptyPool (hex):", buffer_cmd.toString('hex'));
  
  const instruction = new TransactionInstruction(
    {
    keys: [
        {pubkey: manager.publicKey,         isSigner: false, isWritable: false}, // 0
        {pubkey: pool_at_account,           isSigner: false, isWritable: true},  // 1
        {pubkey: pools_manager_account,     isSigner: false, isWritable: true},  // 2
        {pubkey: TOKEN_ID,                  isSigner: false, isWritable: false}, // 3
        {pubkey: UPALA_PROGRAM_ID,          isSigner: false, isWritable: false}, // 4 
        {pubkey: SystemProgram.programId,   isSigner: false, isWritable: false}, // 5
        {pubkey: SYSVAR_RENT_PUBKEY,        isSigner: false, isWritable: false}, // 6
        {pubkey: TOKEN_PROGRAM_ID,          isSigner: false, isWritable: false}, // 7
        {pubkey: user_account.publicKey,    isSigner: true,  isWritable: false}, // 9
        {pubkey: user_at_account,           isSigner: false, isWritable: true}, // 8
      ],
    programId: UPALA_PROGRAM_ID,
    data: buffer_cmd,
  });
  
  let tx = new Transaction().add(instruction);
	tx.recentBlockhash	= (await connection.getRecentBlockhash()).blockhash;
	tx.feePayer			    = user_account.publicKey;
	tx.sign(user_account);

	const signers = [user_account];
	const resultTxSimul = await connection.simulateTransaction(tx, signers);
	const txResponse = resultTxSimul.value;
	
  console.error("Transaction error:", txResponse.err);
  if (txResponse.err == null)
	{
		console.log('Transaction Signature (create associated token account)', 
      await sendAndConfirmTransaction(
        connection,
        tx,
        signers
      ));
	}
	console.log(txResponse.logs);

  return pool_at_account;
}


export async function mintToPool(key:PublicKey): Promise<void>
{
  const minter:Keypair = await readAccountFromFile(TOKEN_KEYPAIR_PATH);
  const payer:Keypair = await readAccountFromFile(TOKEN_PAYER_PATH);
  const token = new Token(connection, minter.publicKey, TOKEN_PROGRAM_ID, minter);
  
  const info:AccountInfo = await token.getAccountInfo(key);
  console.log('Owner of associated token account', info?.owner.toBase58());


  console.log('Transaction Signature (mint to pool)', 
    await token.mintTo(key, minter, [], 1000 * 10e9)
  );
}


// /**
//  * Report the number of times the greeted account has been said hello to
//  */
// export async function reportGreetings(): Promise<void> {
//   const accountInfo = await connection.getAccountInfo(greetedPubkey);
//   if (accountInfo === null) {
//     throw 'Error: cannot find the greeted account';
//   }
//   const greeting = borsh.deserialize(
//     GreetingSchema,
//     GreetingAccount,
//     accountInfo.data,
//   );
//   console.log(
//     greetedPubkey.toBase58(),
//     'has been greeted',
//     greeting.counter,
//     'time(s)',
//   );
// }
// zzz
export function sleep(ms: number): Promise<void> {
  return new Promise(resolve => setTimeout(resolve, ms));
}


/*
export async function create(): Promise<void>
{
  const manager:Keypair = await loadManager();
  const associated_account_address = await createGroupPoolAddress([manager.publicKey, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Associated token account', associated_account_address.toBase58());

  let pool_account_address = await findAssociatedTokenAddress([shittoken, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Pool token account', pool_account_address.toBase58());

//   let source_id = new PublicKey('9DWENSBNh6PvkXvUXMGCFKMeGXiLWVLexcwTg82J4Cs6');


  const buffer_cmd = Buffer.alloc(1);
  buffer_cmd.writeUInt8(UpalaInstution.UI_AddUser, 0);
  const buffer_count = Buffer.alloc(1);
  buffer_count.writeUInt8(1, 0);
  const buffer_score = Buffer.alloc(8);
  buffer_score.writeBigUInt64LE(BigInt(350), 0);
  
  const k = Keypair.generate().publicKey;
  console.log(k.toBase58());
  
  const data = Buffer.concat(
    [
      buffer_cmd,
      new PublicKey('HR2gKCj2rDsK52EQBwXKwJDCVwbUQGichvT9pLvnebkQ').toBuffer(),
      buffer_count,
      k.toBuffer(),
      buffer_score
    ]
  );

  console.log(data.toString('hex'));
  

  const instruction = new TransactionInstruction(
    {
    keys: [
        {pubkey: userAccount.publicKey,     isSigner: true, isWritable: true},   // 0
        {pubkey: associated_account_address,isSigner: false, isWritable: true},  // 1
        {pubkey: shittoken,                 isSigner: false, isWritable: false}, // 2
        {pubkey: UPALA_PROGRAM_ID,          isSigner: false, isWritable: false}, // 3 
        {pubkey: SystemProgram.programId,   isSigner: false, isWritable: false}, // 4
        {pubkey: SYSVAR_RENT_PUBKEY,        isSigner: false, isWritable: false}, // 5
        {pubkey: TOKEN_PROGRAM_ID,          isSigner: false, isWritable: false}, // 6
        {pubkey: accepting,                 isSigner: false, isWritable: true},  // 7
        {pubkey: pool_account_address,      isSigner: false, isWritable: true},  // 7
      ],
    programId: UPALA_PROGRAM_ID,
    data: data,
  });
  
  let tx = new Transaction().add(instruction);
	tx.recentBlockhash	= (await connection.getRecentBlockhash()).blockhash;
	tx.feePayer			    = userAccount.publicKey;
	tx.sign(userAccount);

	const signers = [userAccount];
	const resultTxSimul = await connection.simulateTransaction(tx, signers);
	const txResponse = resultTxSimul.value;
	
  console.log(txResponse.err);
  if (txResponse.err == null)
	{
    console.log("----------");
		console.log('Transaction Signature (create associated token account)', 
      await sendAndConfirmTransaction(
        connection,
        tx,
        signers
      ));
	}
	console.log(txResponse.logs);
  
//   let token = new Token(connection, shittoken, TOKEN_PROGRAM_ID, shittoken_signer);
//   let info:AccountInfo = await token.getAccountInfo(associated_account_address);
//   console.log('Owner of associated token account', info?.owner.toBase58());

  // console.log('Transaction Signature (transfer token)',
  //   await token.transfer(
  //     new PublicKey('9DWENSBNh6PvkXvUXMGCFKMeGXiLWVLexcwTg82J4Cs6'), // from
  //     new PublicKey('BxAbRsfQxuB85FshqdVNnAgmRg8QtX5u2GqCyxi1PZKg'), // to
  //     upala,
  //     [],
  //     17*10e9
  //   )
  // );
}*/