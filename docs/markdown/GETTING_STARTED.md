# Getting Started with FCT_Node

## Installation

### Prerequisites
- CMake 3.20 or later
- C++20 compatible compiler (Visual Studio 2022 recommended)
- npm (optional, for Node.js package management)

### 1. Clone the Repository
```bash
git clone https://github.com/MaaBlock/FCT_Node --recursive
```

### 2. Project Integration
Add to your CMakeLists.txt:
```cmake
add_subdirectory(FCT_Node)
target_link_fct_node_libraries(your_target)
```

### Important Notes
- **Recursive clone is mandatory** - Contains Node.js as submodule
- **C++20 is required** - Node.js core requires modern C++ features
- Automatic configuration via `target_link_fct_node_libraries()`
```
