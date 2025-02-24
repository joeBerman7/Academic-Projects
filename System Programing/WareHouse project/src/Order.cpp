#include "../include/Order.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

using namespace std;

Order::Order(int id, int customerId, int distance) :
    id(id), customerId(customerId), distance(distance), status(OrderStatus::PENDING), collectorId(NO_VOLUNTEER), driverId(NO_VOLUNTEER) {}

Order *Order::clone() const
{
    return new Order(*this);
}

int Order::getId() const
{
    return id;
}

int Order::getCustomerId() const
{
    return customerId;
}

void Order::setStatus(OrderStatus status) 
{
    this->status = status;
}

void Order::setCollectorId(int collectorId)
{
    this->collectorId = collectorId;
}

void Order::setDriverId(int driverId)
{
    this->driverId = driverId;
}

int Order::getCollectorId() const 
{
    return collectorId;
}

int Order::getDriverId() const
{
    return driverId;
}

OrderStatus Order::getStatus() const{
    return status;
}


int Order::getDistance() const
{
    return distance;
}   

//change with collector/driver 
const string Order::toString() const 
{
    string statusStr, colId, drivId;
    if(status == OrderStatus::PENDING){statusStr = "PENDING";}
    if(status == OrderStatus::COLLECTING){statusStr = "COLLECTING";}
    if(status == OrderStatus::DELIVERING){statusStr = "DELIVERING";}
    if(status == OrderStatus::COMPLETED){statusStr = "COMPLETED";}
    if(collectorId == -1){colId = "None";}else{colId = to_string(collectorId);}
    if(driverId == -1){drivId = "None";}else{drivId = to_string(driverId);}
    
    return "OrderId: " + to_string(id) + "\n" +
           "OrderStatus: " + statusStr + "\n" +
           "CustomerID: " + to_string(customerId) + "\n" +
           "Collector: " + colId + "\n" +
           "Driver: " + drivId;
}

