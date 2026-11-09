# UltraCalc API
### Let there be calculator

There are 2 ways to use the API
1. In Frontends
2. Feature Extensions 

## Frontends
Using the UltraCalc API in your frontend allows you to use extensions with it

Getting started is simple
```bash ultracalc --calcapi mul 8 7```
You can get the result of an equation by running the calcapi flag with your operator and 2 numbers. 
like this: ultracalc --calcapi [operator] (number1) (number2)

### Operators available:
- `add`: Addition
- `sub`: Subtraction
- `mul`: Multiplication
- `div`: Division
- `mod`: Modulo

The app returns the answer in plain text. 

You can also use the FrontLang Wizard. 
With it, you can drag buttons onto a canvas, set your backgrounds, and it'll parse to an extension.