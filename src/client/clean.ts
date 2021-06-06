/**
 * Clean data of the upala group
 */
import { PublicKey } from '@solana/web3.js';
import {
  cleanStorage,
  establishConnection,
  loadProgramId,
  loadTokenId,
} from './lib';

async function main() {
  console.log("#CLEAN_DATA_GROUPS");
  await establishConnection();
  await loadProgramId();
  await loadTokenId();
  await cleanStorage();
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
