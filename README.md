## Simple blockchain on CLI

- How it works
  - Server must be on first with `server.sh` and next one is `client.sh`
  - Client has three options: make a block, see all the blocks, and quit
  - One block is consist of
    1. Current hash
    2. Previous hash
    3. Height
    4. Inputs
    5. User name 
  - Block will save as one file and the most recent block will be written twice
  - Current hash will be the file name and there is one file called `blockchain` for saving a hash of recent one
  - All the blocks are saved on a blockchain folder in a server
  - Reference: [blockchain explorer](https://www.blockchain.com/btc/blocks?page=1)
- Must-included elements
  - [x] Shell script: `server.sh` and `client.sh`
  - [ ] Timer signal
  - [ ] Keyboard signal
  - [x] Use `fork`
  - [ ] Pipe
  - [x] Socket
  - [ ] Thread
