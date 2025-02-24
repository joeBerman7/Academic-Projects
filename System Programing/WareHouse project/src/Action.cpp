#include <string>
#include <vector>
#include <iostream>
#include "../include/WareHouse.h"
#include "../include/Action.h"
#include "../include/Customer.h"
#include "../include/Volunteer.h"
#include "../include/Order.h"

using std::string;
using std::vector;
using namespace std;

extern WareHouse* backup;
/////////////////////////////////////////////////////

// BaseAction Implementation

BaseAction::BaseAction() : errorMsg(""),status(ActionStatus::COMPLETED) {}
BaseAction::~BaseAction() {}

ActionStatus BaseAction::getStatus() const
{
    return status;
}

void BaseAction::complete()
{
    status = ActionStatus::COMPLETED;
}


void BaseAction::error(string errorMsg) 
{
    status = ActionStatus::ERROR;
    this->errorMsg = std::move(errorMsg);
}

string BaseAction::getErrorMsg() const 
{
    return errorMsg;
}

// SimulateStep Implementation ////////////////////////////////////////////////

SimulateStep::SimulateStep(int numOfSteps) : numOfSteps(numOfSteps) {}
SimulateStep::~SimulateStep() {}

void SimulateStep::act(WareHouse &wareHouse) 
{
    wareHouse.simulateStep(numOfSteps, this);
    complete();
}

string SimulateStep::toString() const 
{
    string statusStr;
    if(getStatus() == ActionStatus::COMPLETED){statusStr = "COMPLETED";}
    if(getStatus() == ActionStatus::ERROR){statusStr = "ERROR";}
    return "simulateStep " + to_string(numOfSteps) + " " + statusStr;
}

SimulateStep *SimulateStep::clone() const 
{
    return new SimulateStep(*this);
}

// AddOrder Implementation ////////////////////////////////////////////////////

AddOrder::AddOrder(int id) : customerId(id) {}
AddOrder::~AddOrder() {}

void AddOrder::act(WareHouse &wareHouse) 
{
    Order* newOrder;
    error("cannot place this order");
    if (wareHouse.getCustomer(customerId).getId() == -1)
    {
        error("Cnnot place this order");
        cout << getErrorMsg() << endl;
    }
    else if(!wareHouse.getCustomer(customerId).canMakeOrder())
    {
        error("Cnnot place this order");
        cout << getErrorMsg() << endl;
    }
    else
    {
        int newDist = wareHouse.getCustomer(customerId).getCustomerDistance();
        newOrder = new Order(wareHouse.getOrdersCounter(), customerId, newDist);
        wareHouse.getCustomer(customerId).addOrder(wareHouse.getOrdersCounter());
        wareHouse.increaseOrdersCounter();
        wareHouse.addOrder(newOrder);
        complete();
    }
}

string AddOrder::toString() const 
{
    string statusStr;
    if(getStatus() == ActionStatus::COMPLETED){statusStr = "COMPLETED";}
    if(getStatus() == ActionStatus::ERROR){statusStr = "ERROR";}
    return "order " + to_string(customerId) + " " + statusStr;
}

AddOrder *AddOrder::clone() const {
    return new AddOrder(*this);
}


// AddCustomer Implementation /////////////////////////////////////////////////////////

AddCustomer::AddCustomer(const string &customerName, const string &customerType, int distance, int maxOrders)
    : customerName(customerName), customerType((customerType == "Soldier") ? CustomerType::Soldier : CustomerType::Civilian),
      distance(distance), maxOrders(maxOrders) 
      {}

AddCustomer::~AddCustomer() {}

void AddCustomer::act(WareHouse &wareHouse) 
{
    Customer* newCustomer;
    int cusId;
    if (customerType == CustomerType::Soldier){
        int cusId = wareHouse.getCustomersCounter();
        newCustomer = new SoldierCustomer(cusId, customerName, distance, maxOrders);
    }
    else{
        cusId = wareHouse.getCustomersCounter();
        newCustomer = new CivilianCustomer(cusId, customerName, distance, maxOrders);
    }
    wareHouse.addCustomer(newCustomer);
    wareHouse.increaseCustomersCounter();
    complete();
}

string AddCustomer::toString() const 
{
    string statusStr;string cusType;
    if(getStatus() == ActionStatus::COMPLETED){statusStr = "COMPLETED";}
    if(getStatus() == ActionStatus::ERROR){statusStr = "ERROR";}
    if(customerType == CustomerType::Civilian){cusType = "Civilian";}
    if(customerType == CustomerType::Soldier){cusType = "Soldier";}

    return 
    "customer " + 
    customerName + " " + 
    cusType + " " + 
    to_string(distance) + " " +
    to_string(maxOrders) + " " + 
    statusStr;
}


AddCustomer *AddCustomer::clone() const {
    return new AddCustomer(*this);
}


// PrintOrderStatus Implementation /////////////////////////////////////////////////////////

PrintOrderStatus::PrintOrderStatus(int id) : orderId(id) {}
PrintOrderStatus::~PrintOrderStatus() {}

void PrintOrderStatus::act(WareHouse &wareHouse) 
{
    string orderStatusStr;
    if(wareHouse.getOrder(orderId).getId() == -1)
    {
        error("order doesen't exist");
        cout << getErrorMsg() << endl;
    }
    else{
        cout << wareHouse.getOrder(orderId).toString() << endl;
        complete();
    }
}

PrintOrderStatus *PrintOrderStatus::clone() const 
{
    return new PrintOrderStatus(*this);
}

string PrintOrderStatus::toString() const 
{
    string statusStr;
    if(getStatus() == ActionStatus::COMPLETED){statusStr = "COMPLETED";}
    if(getStatus() == ActionStatus::ERROR){statusStr = "ERROR";}
    return "OrderStatus " + to_string(orderId) + " " + statusStr;
}


// PrintCustomerStatus Implementation /////////////////////////////////////////////////////////

PrintCustomerStatus::PrintCustomerStatus(int customerId) : customerId(customerId) {}

PrintCustomerStatus::~PrintCustomerStatus() {}

void PrintCustomerStatus::act(WareHouse &wareHouse) 
{
    if (wareHouse.getCustomer(customerId).getId() == -1)
    {   
        error("Customer doesn't exist");
        cout << getErrorMsg() << endl;
    }
    else{
        const vector<int> &currCusOrders = wareHouse.getCustomer(customerId).getOrdersIds();
        cout << "CustomerID:" << customerId << endl;
        for(int ids : currCusOrders){
            string customerOrderStatusStr;
            Order &currCusOrder = wareHouse.getOrder(ids);
            switch (currCusOrder.getStatus()) {
                case OrderStatus::PENDING:
                    customerOrderStatusStr = "PENDING";
                    break;
                case OrderStatus::COLLECTING:
                    customerOrderStatusStr = "COLLECTING";
                    break;
                case OrderStatus::DELIVERING:
                    customerOrderStatusStr = "DELIVERING";
                    break;
                case OrderStatus::COMPLETED:
                    customerOrderStatusStr = "COMPLETED";
                    break;
            }

            cout << "OrderID:" << ids << endl;
            cout << "OrderStatus:" << customerOrderStatusStr << endl;
        }
        cout << "numOrdersLeft:" << wareHouse.getCustomer(customerId).getMaxOrders() - wareHouse.getCustomer(customerId).getNumOrders()<< endl;        
    }
    complete();
}

PrintCustomerStatus *PrintCustomerStatus::clone() const 
{
    return new PrintCustomerStatus(*this);
}

string PrintCustomerStatus::toString() const 
{   
    string customerStatusStr;
    if(getStatus() == ActionStatus::COMPLETED){customerStatusStr = "COMPLETED";}
    if(getStatus() == ActionStatus::ERROR){customerStatusStr = "ERROR";}
    return "CustomerStatus " + to_string(customerId) + " " + customerStatusStr;
}

// PrintVolunteerStatus Implementation /////////////////////////////////////////////////////////

PrintVolunteerStatus::PrintVolunteerStatus(int id) : volunteerId(id) {}
PrintVolunteerStatus::~PrintVolunteerStatus() {}

void PrintVolunteerStatus::act(WareHouse &wareHouse) 
{
    if (wareHouse.getVolunteer(volunteerId).getId() == -1)
    {
        error("Volunteer dosen't exist");
        cout << getErrorMsg() << endl;
    }
    else{
        Volunteer &currVol = wareHouse.getVolunteer(volunteerId);
        string busy;
        if(currVol.isBusy()){busy = "True";}
        else{busy = "False";}
        cout << "VolunteerID:" << volunteerId << endl;
        cout << "isBusy: " << busy << endl;
        cout << "OrderId: " << currVol.getActiveOrderId() << endl; 
        if(typeid(currVol) == typeid(CollectorVolunteer))
        {
           cout << "TimeLeft: " << dynamic_cast<CollectorVolunteer&>(currVol).getTimeLeft() << endl;
           cout << "ordersLeft: No Limit" << endl;
        }
        else if (typeid(currVol) == typeid(LimitedCollectorVolunteer)) {
            cout << "TimeLeft:" << dynamic_cast<LimitedCollectorVolunteer&>(currVol).getTimeLeft() << endl;
            cout << "ordersLeft:" << dynamic_cast<LimitedCollectorVolunteer&>(currVol).getNumOrdersLeft() << endl;
        }
        else if(typeid(currVol) == typeid(DriverVolunteer))
        {
           cout << "distanceLeft:" << dynamic_cast<DriverVolunteer&>(currVol).getDistanceLeft() << endl;
           cout << "ordersLeft: No Limit" << endl;
        }
        else if (typeid(currVol) == typeid(LimitedDriverVolunteer)) {
            cout << "distanceLeft:" << dynamic_cast<LimitedCollectorVolunteer&>(currVol).getTimeLeft() << endl;
            cout << "ordersLeft:" << dynamic_cast<LimitedCollectorVolunteer&>(currVol).getNumOrdersLeft() << endl;
        }
    }
    complete();
}

PrintVolunteerStatus *PrintVolunteerStatus::clone() const 
{
    return new PrintVolunteerStatus(*this);
}

string PrintVolunteerStatus::toString() const 
{   
    string VolunteerStatusStr;
    if(getStatus() == ActionStatus::COMPLETED){VolunteerStatusStr = "COMPLETED";}
    if(getStatus() == ActionStatus::ERROR){VolunteerStatusStr = "ERROR";}
    return "VolunteerStatus " + VolunteerStatusStr;
}

// PrintActionsLog Implementation /////////////////////////////////////////////////////////

PrintActionsLog::PrintActionsLog(){}
PrintActionsLog::~PrintActionsLog(){}

void PrintActionsLog::act(WareHouse &wareHouse)
{
    const vector<BaseAction*> &actions = wareHouse.getActions();
    for (auto &action : actions){cout << action->toString() << endl;}
    complete();
}

PrintActionsLog *PrintActionsLog::clone() const
{
    return new PrintActionsLog(*this);
}

string PrintActionsLog::toString() const
{
    return "log COMPLETED";
}


// Close Implementation /////////////////////////////////////////////////////////

Close::Close(){}

void Close::act(WareHouse &wareHouse)
{
    for(auto &order : wareHouse.getPendingOrders())
    {
        string orderStat;
        if(order->getStatus() == OrderStatus::COLLECTING){orderStat = "COLLECTING";}
        if(order->getStatus() == OrderStatus::COMPLETED){orderStat = "COMPLETED";}
        if(order->getStatus() == OrderStatus::DELIVERING){orderStat = "DELIVERING";}
        if(order->getStatus() == OrderStatus::PENDING){orderStat = "PENDING";}
        cout<< "OrderID:" <<order->getId() << ", CustomerID" << order->getCustomerId() << " OrderStatus:"<< orderStat << endl;
    }
    for(auto &order : wareHouse.getInProcessOrders())
    {
        string orderStat;
        if(order->getStatus() == OrderStatus::COLLECTING){orderStat = "COLLECTING";}
        if(order->getStatus() == OrderStatus::COMPLETED){orderStat = "COMPLETED";}
        if(order->getStatus() == OrderStatus::DELIVERING){orderStat = "DELIVERING";}
        if(order->getStatus() == OrderStatus::PENDING){orderStat = "PENDING";}
        cout<< "OrderID:" <<order->getId() << ", CustomerID" << order->getCustomerId() << " OrderStatus:"<< orderStat << endl;
    }
    for(auto &order : wareHouse.getCompletedOrders())
    {
        string orderStat;
        if(order->getStatus() == OrderStatus::COLLECTING){orderStat = "COLLECTING";}
        if(order->getStatus() == OrderStatus::COMPLETED){orderStat = "COMPLETED";}
        if(order->getStatus() == OrderStatus::DELIVERING){orderStat = "DELIVERING";}
        if(order->getStatus() == OrderStatus::PENDING){orderStat = "PENDING";}
        cout<< "OrderID:" <<order->getId() << ", CustomerID" << order->getCustomerId() << " OrderStatus:"<< orderStat << endl;
    }
    wareHouse.close();
    complete();
}

Close *Close::clone() const
{
    return new Close(*this);
}

string Close::toString() const
{
    return "";
}

// BackupWareHouse Implementation /////////////////////////////////////////////////////////

BackupWareHouse::BackupWareHouse(){}

void BackupWareHouse::act(WareHouse &wareHouse)
{
    if(backup != nullptr){delete backup;}
    else{backup = new WareHouse(wareHouse);} 
    complete();
}

BackupWareHouse *BackupWareHouse::clone() const
{
    return new BackupWareHouse(*this);
}

string BackupWareHouse::toString() const
{
    string statusStr;
    if(this->getStatus() == ActionStatus::COMPLETED){statusStr="COMPLETED";}
    else if(this->getStatus() == ActionStatus::ERROR){statusStr="ERROR";}
    return "Backup" + statusStr;
}


// RestoreWareHouse Implementation /////////////////////////////////////////////////////////

RestoreWareHouse::RestoreWareHouse(){}

void RestoreWareHouse::act(WareHouse &wareHouse)
{
    if(backup == nullptr){
        error("No backup available");
        cout << getErrorMsg() << endl;
    }
    else{
        wareHouse = *backup;
        complete();
    }
}

RestoreWareHouse *RestoreWareHouse::clone() const
{
    return new RestoreWareHouse(*this);
}

string RestoreWareHouse::toString() const
{
    string statusStr;
    if(this->getStatus() == ActionStatus::COMPLETED){statusStr="COMPLETED";}
    else if(this->getStatus() == ActionStatus::ERROR){statusStr="ERROR";}
    return "Restore " + statusStr;
}
