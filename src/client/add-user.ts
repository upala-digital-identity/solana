/**
 * Added user to the upala group
 */
import { Keypair, PublicKey } from '@solana/web3.js';
import {
  addUser,
  createGroupPoolAddress,
  establishConnection,
  loadManager,
  loadProgramId,
  loadTokenId,
  TOKEN_ID,
  UPALA_PROGRAM_ID,
  USER_1_KEYPAIR_PATH
} from './lib';
import { readAccountFromFile } from './utils';

async function main() {
  console.log("#ADD_USER_TO_GROUP");
  await establishConnection();
  await loadProgramId();
  await loadTokenId();
  const user:Keypair = await readAccountFromFile(USER_1_KEYPAIR_PATH);
  console.log(user.publicKey.toBase58());

  // const group_id = new PublicKey('9ZyEEr7YcDA6voLgD2a5kXEZbhF5yG7L5Qw4ZEhKatjB');
  const manager:Keypair = await loadManager();
  const group_id:PublicKey = await createGroupPoolAddress([manager.publicKey, TOKEN_ID, UPALA_PROGRAM_ID], UPALA_PROGRAM_ID);
  console.log('Associated token account:', group_id.toBase58());

  await addUser(group_id, user.publicKey, 10);
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
