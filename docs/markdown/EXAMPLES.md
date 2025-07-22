```markdown
# FCT_Node Examples

## Basic Interoperation
```c++
// C++ function callable from JavaScript
env.global()["cppAdd"] = [](int a, int b) { return a + b; };

// Call JavaScript from C++
auto result = env.callFunction<std::string>("jsFunction");
```

## Object Manipulation
```c++
JSObject user = env.callFunction<JSObject>("createUser", "Alice", 25);
string name = user["name"];  // Access properties
user["age"] = 26;           // Modify properties
```

## Asynchronous Operations
```c++
auto promise = env.callFunction<JSPromise>("asyncOperation");
while (!promise.isSettled()) {
    env.tick();
    std::this_thread::sleep_for(10ms);
}
```

## Complete Example
See [tutorial repository](https://github.com/MaaBlock/FCT_NodeTutorial) for complete examples including:
- AI service integration
- Web API consumption
- Complex object serialization
```
