<p align="center">
  <a href="https://solana.com">
    <img alt="Solana" src="https://i.imgur.com/uBVzyX3.png" width="250" />
  </a>
</p>

# Hello world on Solana+Upala

This project demonstrates how to use the `Upala` to
interact with programs on the Solana blockchain.

The project comprises of:

* An on-chain upala program on C
* A client that can send:
  * `npm run create-group` for the create new Upala group
  * `npm run add-user` for adding the new user to Upala group
  * `npm run empty-pool` to go out with the bank from Upala group
  * `npm run remove-groups` a simple clean the program storage

## Table of Contents
- [Hello world on Solana](#hello-world-on-solana)
  - [Table of Contents](#table-of-contents)
  - [Quick Start](#quick-start)
    - [Build the on-chain program](#build-the-on-chain-program)
    - [Run the client](#run-the-client)
  - [Restarting](#restarting)
  - [Example signatures on the testnet](#example-signatures-on-the-testnet)


## Quick Start

The following dependencies are required to build and run this example, depending
on your OS, they may already be installed:

- Install node (v14 recommended)
- Install npm
- Install the latest Rust stable from https://rustup.rs/
- Install Solana v1.6.6 or later from
  https://docs.solana.com/cli/install-solana-cli-tools

If this is your first time using Rust, these [Installation
Notes](README-installation-notes.md) might be helpful.


### Install npm dependencies

```bash
$ npm install
```

### Build the on-chain program

```bash
$ npm run build:program-c
```

### Deploy the on-chain program

```bash
$ solana program deploy dist/program/helloworld.so
```

### Run the client

```bash
$ npm run create-group | add-user | empty-pool | remove-groups
```

## Restarting

```
$ npm run clean
$ npm run build:program-c
$ solana -C wallet/config.yml program deploy dist/program/helloworld.so
```


## Example signatures on the testnet

**create-group**
```
Fej4ueDBtKnCC7R56hsPu4tMnXvPxhspehMi7FRcojgRrcAHR2sMLG8yuG2d92hgwnUM998Vu5NhbAbPUe1u2bv
```

**add-user**
```
5SvKxDnTzVSYbv47uWBhsNGBzmU714UyoL5Vp3tgKk1vMV23YeD4h7tBn89AGpPJbXBUAvVUnr3DBNiwzLDs1gHF
```

**empty-pool**
```
4w2ctYbupZjBKKW8Z2CptLTLW9MgostXyVAwtHYkvBFKzuf2ckqrEzXUaiLzqDe5NRCpm1sZgNDasRXm1uF6ite4
```

**remove-groups**
```
TRiz22dYed8xjPvowExkxGZZEPvKwyjaGf4xgh7ZWek8PrbNY8259S6TmBaFNycA5qCvFWvpJqMXcdNy7syRPqL
```

