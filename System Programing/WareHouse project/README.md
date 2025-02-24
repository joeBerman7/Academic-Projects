# Food Warehouse Management System with Memory Management in C++

## Introduction

This project simulates a **Food Warehouse Management System** where volunteers and customers interact, and orders are managed. The system needs to handle various aspects of warehouse operations, including assigning volunteers, processing customer orders, and more. 

The primary focus of this assignment is to implement the system using **C++**, with an emphasis on **memory management** to ensure the program is both efficient and free from common issues like memory leaks.

### Why Memory Management in C++?

Memory management in C++ is a critical aspect of writing efficient and stable programs. Unlike higher-level programming languages, C++ gives the programmer direct control over memory allocation and deallocation, which allows for optimal resource usage but also demands careful attention to avoid errors such as memory leaks, dangling pointers, and undefined behavior.

In C++, memory is typically managed in two main areas:

1. **Stack Memory**: Automatically managed, used for local variables and function calls.
2. **Heap Memory**: Managed manually using `new` and `delete`, used for objects that need to persist beyond function calls.

### Importance of Memory Management

Proper memory management is essential for the following reasons:
- **Preventing Memory Leaks**: Failure to free dynamically allocated memory results in memory leaks, causing the program to use excessive resources.
- **Optimizing Performance**: Efficient memory usage allows for handling larger datasets and improving execution speed.
- **Avoiding Undefined Behavior**: Improper memory handling can lead to undefined behavior and crashes.
- **Resource Efficiency**: Proper management makes your program efficient, especially on systems with limited resources.

---

## Project Overview: Food Warehouse Management System

In this project, you are tasked with simulating a **food warehouse management system** for a country called SPLand. The warehouse serves food packages to citizens and manages customer orders with the help of volunteers. The warehouse system will simulate:

- Assigning roles to volunteers.
- Managing customer orders.
- Simulating the processing of orders and customer interactions.

### Memory Management in the Project

Efficient memory management is crucial for handling large numbers of orders, customers, and volunteers in this simulation. Here’s how memory management applies to different aspects of this project:

### 1. **Dynamic Memory Allocation**

As customers place orders, **dynamic memory allocation** is required to handle each `Order` object. You will use `new` to allocate memory for these objects, and `delete` to free them once the order has been processed.

### 2. **Using Classes and the Rule of 5**

In the Food Warehouse Management System, you will be designing classes such as:

- **Order**: Represents a customer's order, including details like items and quantities.
- **Customer**: Represents a customer, managing their personal data and order history.
- **Volunteer**: Represents volunteers who are processing orders.

Each class should define the following to handle memory properly:
- **Destructor**: Releases dynamically allocated memory (e.g., for orders that are completed).
- **Copy Constructor**: Correctly handles copying objects, ensuring no memory conflicts.
- **Move Constructor**: Efficiently moves resources when an object is passed by value.
- **Copy Assignment Operator**: Ensures proper handling of memory when one object is assigned to another.
- **Move Assignment Operator**: Optimizes resource transfer during assignments.

### 3. **Avoiding Memory Leaks**

You must ensure that any memory dynamically allocated for objects like `Order` and `Volunteer` is properly freed. If an object is no longer needed, its destructor will be called to release the allocated memory. If the memory is not properly released, the system will accumulate memory leaks, which could lead to performance degradation or even system crashes over time.

### 4. **Efficient Resource Management**

Since the program may handle multiple orders, volunteers, and customers, it's important to keep memory management efficient. If an order is completed and removed from the system, the associated memory must be freed immediately.

You can also use **smart pointers** (`std::unique_ptr` or `std::shared_ptr`) to automatically manage memory. This helps reduce the risk of forgetting to free memory manually.

### Example: Managing Orders in the Warehouse

Here’s an example of how memory management works for orders in the system:

- **Creating an Order**:
   - When a customer places an order, memory is allocated for the `Order` object using `new`.
   
- **Copying an Order**:
   - If an order needs to be copied (e.g., transferring from one list to another), the **copy constructor** will handle the memory allocation and prevent conflicts.
   
- **Moving an Order**:
   - If an order is moved (e.g., transferring ownership), the **move constructor** or **move assignment operator** ensures resources are transferred efficiently.
   
- **Deleting an Order**:
   - Once an order is completed, its memory is freed using `delete` or through a smart pointer, preventing memory leaks.

---

## Features

1. **Order Management**: Handles the creation, deletion, and transfer of orders, ensuring proper memory management throughout.
2. **Volunteer Management**: Manages volunteer data and assigns them tasks.
3. **Efficient Memory Use**: Implements memory management best practices to avoid leaks and optimize performance.
4. **Object-Oriented Design**: Utilizes classes to model the warehouse system, employing the **Rule of 5** to manage resources correctly.

