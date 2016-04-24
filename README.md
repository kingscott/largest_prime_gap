# cp331_a1

## compile

If you are running the program on your local machine using OpenMPI, the commands are as follows:

```bash
➜  ~ mpicc {name}.c -o {executable_name} -lmpi -lm -gmp
➜  ~ mpirun -np {num_procs} ./{executable_name}
```
