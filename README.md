# Semaphores_post_office
Script is a simulation of a post office using processes and semaphores in C. The program models the interactions between customers and post office workers, including queue management, service handling and synchronized operations.

## Compilation 
- ensure `Makefile` is in the same directory as `proj2.c`
- compile program using command `make` 

## Usage
```bash
  ./proj2 <NZ> <NU> <TZ> <TU> <F>
```
### Parameters
`NZ`: number of customers<br>
`NU`: number of post office workers<br>
`TZ`: maximum customer delay before entering the post office (range <0, 10000> in ms)<br>
`TU`: maximum post office worker break time (range <0, 100> in ms)<br>
`F`: maximum time before the post office closes (range <0, 10000> in ms)<br>

## Output
Program outputs logs to a `.out` file detailing the whole process
