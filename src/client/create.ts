/**
 * Create upala group
 */
import { PublicKey } from '@solana/web3.js';
import {
  create,
  establishConnection,
  loadProgramId,
  loadTokenId,
  mintToPool,
} from './lib';

async function main() {
  console.log("#CREATE_GROUP");
  await establishConnection();
  await loadProgramId();
  await loadTokenId();
  const ata:PublicKey = await create();
  // await mintToPool(ata);
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
