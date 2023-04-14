#4)They're all 8 bytes.

#5)Yes, this is because simply contains the address of a variable, and the pointer itself has an address.
    int *p = &p;

#6)When you compare p to the null char, p itself holds an address, not a character, so the comparison wouldn't work. To fix it, you need to do *p = '\0'

#7)Arrays are similar to pointers because C treats array variables as pointers. However, the difference lies in that pointers can point to any address while the array variable can only point to the starting address of the array, and it can't be changed.