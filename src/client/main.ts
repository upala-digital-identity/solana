/**
 * Hello world
 */
import {
  createSPLAccount,
  establishConnection,
  establishUsers,
  printPubkey,
} from './lib';

async function main() {
  console.log("Let's say hello to a Solana account...");

  // Establish connection to the cluster
  // await establishConnection();
  // await establishUsers();
  // await createSPLAccount();
  // console.log(await printPubkey('5HhVfJvcRWk7LSr2hTEvRRej7gQvHd2XL55hP7txThwa'));

  // Check if the program has been deployed
  // await checkProgram();
  
  // // Say hello to an account
  // await sayHello();

  // // Find out how many times that account has been greeted
  // await reportGreetings();

  console.log('Success');
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
