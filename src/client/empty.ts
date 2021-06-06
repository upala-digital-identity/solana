/**
 * Empty pool of upala group
 */
import { Keypair, PublicKey } from '@solana/web3.js';
import {
  empty,
  establishConnection,
  loadProgramId,
  loadTokenId,
  USER_1_KEYPAIR_PATH,
} from './lib';
import { readAccountFromFile } from './utils';

async function main() {
  console.log("#EMPTY_GROUP");
  await establishConnection();
  await loadProgramId();
  await loadTokenId();
  const user:Keypair = await readAccountFromFile(USER_1_KEYPAIR_PATH);
  console.log(user.publicKey.toBase58());

  await empty(user);
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
