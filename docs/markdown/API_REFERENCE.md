# FCT_Node API Reference

## Core Classes

### NodeEnvironment
Main interface for Node.js interaction.

#### Key Methods:
- `setup()` - Initialize the environment
- `stop()` - Pause execution
- `addModulePath(path)` - Add NPM module search path
- `executeScript(code)` - Run JavaScript code
- `callFunction<T>(name, ...args)` - Call JS function

### JSObject
Wrapper for JavaScript objects with type-safe access.

### JSPromise
Handle asynchronous operations with Promise support.

## Basic Usage Pattern
```c++
#include <FCT_Node.h>

int main() {
    NodeCommon::Init();
    NodeEnvironment env;
    
    env.addModulePath("./node_modules");
    env.setup();
    
    // Interoperation code here
    
    env.stop();
    NodeCommon::Term();
    return 0;
}
```