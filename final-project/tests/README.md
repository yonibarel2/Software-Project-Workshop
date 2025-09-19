# Test Guide – SymNMF Project

This guide describes how to test the **input validation** and functionality of the SymNMF project step by step.  
The first stage focuses on ensuring the program correctly reads and rejects invalid CSV input files.

---

## 1. CSV Points File (tests relevant for: `symnmf.c`, `symnmf.py`): 

The program expects a text file containing points in **CSV format**:

- Each row has the form: `float,float,float,...,float\n`
- This format applies to every row, including the last row in the file (the file must end with a newline, not with the last number).  
- Each coordinate must be **convertible to a floating-point number** (`float`).  
- Example: `1`, `1.0`, or `3.14159` are all valid (all parsed as floats).  
- All rows must have the **same dimension** (same number of floats).  
- No extra spaces, no trailing commas, no empty lines in between.  

**Example of correct format:**
```txt
1.0,2.0,3.0/n
4,5,6/n
7.25,8.5,9.75/n
```

### ✅ Valid Format Test

**1. Basic ponts file**

```txt
1.0,2.0,3.0\n
4.0,5.0,6.0\n
7.0,8.0,9.0\n
```

**Expected result:** Program accepts input and constructs the matrix successfully.

### ❌ Invalid Format Tests

**1. File not found**: Provide a non-existent path.

**2. Empty file – No rows at all.**: 

```txt
```

**3. Inconsistent dimensions:**
```txt
1.0,2.0,3.0\n
4.0,5.0\n
```

**4. Non-numeric values:**

```txt
1.0,2.0,three\n
4.0,5.0,6.0\n
```
**5. Trailing comma:**

```txt
1.0,2.0,3.0,\n
4.0,5.0,6.0\n
```

**6. Extra spaces:**

```txt
1.0, 2.0, 3.0\n
4.0,5.0,6.0\n
```

**7. Single point only**

```txt
1.0,2.0,3.0\n
```

**8. Blank line in the middle or at the end:**

```txt
1.0,2.0,3.0\n\n
4.0,5.0,6.0\n
```

**Expected result:** Program prints `An Error Has Uccored` an error and exits.

### ❌ ⚠️ Edge Case Tests (Valid Format but Unsupported)

**1. Very large numbers (overflow risk) - program not expected to support:**

```txt
1e308,2e308,3.0\n
4.0,5.0,6.0\n
```

**2. Very small numbers (underflow risk) - program not expected to support:**

```txt
1e-308,2e-308,3e-308\n
4.0,5.0,6.0\n
```

**3. Special values (NaN, inf):**

```txt
NaN,2.0,3.0\n
4.0,inf,6.0\n
```
