#include <string>
#include <vector>
#include "../include/Customer.h"
using std::string;
using std::vector;


Customer::Customer(int id, const string &name, int locationDistance, int maxOrders) : 
    id(id) , name(name), locationDistance(locationDistance) , maxOrders(maxOrders), ordersId() {};
    
Customer::~Customer() {}

const string &Customer::getName() const 
{
    return name;
}

int Customer::getId() const
{
    return id;
}

int Customer::getCustomerDistance() const
{   
    return locationDistance;
}

//Returns maxOrders
int Customer::getMaxOrders() const 
{   
   return maxOrders;
}

//Returns num of orders the customer has made so far
int Customer::getNumOrders() const
{   
    return ordersId.size();
    
}

//Returns true if the customer didn't reach max orders
bool Customer::canMakeOrder() const 
{   
    return getNumOrders() < maxOrders;
}

const vector<int> &Customer::getOrdersIds() const
{
    return const_cast<vector<int> &>(ordersId);
}


//return OrderId if order was added successfully, -1 otherwise
int Customer::addOrder(int orderId)
{
    if (canMakeOrder()) {
        ordersId.push_back(orderId);
        return orderId;
    } else {
        return -1; // Unable to add order
    }
}


// SoldierCustomer class implementation

SoldierCustomer::SoldierCustomer(int id, const string &name, int locationDistance, int maxOrders) 
    : Customer(id, name, locationDistance, maxOrders){}

SoldierCustomer *SoldierCustomer::clone() const 
{
    return new SoldierCustomer(*this);
}


// CivilianCustomer class implementation

CivilianCustomer::CivilianCustomer(int id, const string &name, int locationDistance, int maxOrders)
    : Customer(id, name, locationDistance, maxOrders) {}


CivilianCustomer *CivilianCustomer::clone() const 
{
    return new CivilianCustomer(*this);
}

