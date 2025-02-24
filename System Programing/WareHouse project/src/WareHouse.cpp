#include <string>
#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>
#include <ios>

#include "../include/WareHouse.h"
#include "../include/Order.h"
#include "../include/Customer.h"
#include "../include/Volunteer.h"
#include "../include/Action.h"

#include <algorithm>

using namespace std;

WareHouse::WareHouse(const string &configFilePath)
    : isOpen(true), actionsLog(), volunteers(), pendingOrders(), inProcessOrders(), completedOrders(), customers(), 
    customerCounter(0), volunteerCounter(0) , ordersCounter(0),
    defaultCustomer(new CivilianCustomer(-1, "default", 0, 0)),
    defaultVolunteer(new CollectorVolunteer(-1, "default", 0)),
    defaultOrder(new Order(-1, -1, 0))
{
    parsingTxt(configFilePath);
}

void WareHouse::start() 
{
	cout << "Warehouse is open!" <<endl;
    string userInput;
    while (isOpen) {
        getline(cin, userInput);
        istringstream iss(userInput);
        string command;
        int customerID;
        int orderID;
        int volunteerID;
        int numOfSteps;
        iss >> command;
        if (command == "order") {
            iss >> customerID;
            AddOrder* newOrder = new AddOrder(customerID);
            newOrder->act(*this);
            this->addAction(newOrder);
        } 
        else if (command == "step") {
            iss >> numOfSteps;
            SimulateStep* simulateStep = new SimulateStep(numOfSteps);
            simulateStep->act(*this);
            this->addAction(simulateStep);
        } 
        else if (command == "customer") {
            string name;
            cin >> name;
            string type;
            iss >> type;
            string dist;
            int distance;
            iss >> distance;
            cout << distance<< endl;
            int maxOrders;
            iss >> maxOrders;
            AddCustomer* newCustomer = new AddCustomer(name, type, distance, maxOrders);
            newCustomer->act(*this);
            this->addAction(newCustomer);
        } 
        else if (command == "log") {
            PrintActionsLog* printActionsLog = new PrintActionsLog();
            printActionsLog->act(*this);
            this->addAction(printActionsLog);
        } 
        else if (command == "close") {
            Close* close = new Close();
            close->act(*this);
            this->addAction(close);
            isOpen = false;
            break;
        }   
        else if (command == "orderStatus") {
            iss >> orderID;
            PrintOrderStatus* orderStatus = new PrintOrderStatus(orderID);
            orderStatus->act(*this);
            this->addAction(orderStatus);            
        } 
        else if (command == "volunteerStatus") {
            iss >> volunteerID;
            PrintVolunteerStatus* volunteerStatus = new PrintVolunteerStatus(volunteerID);
            volunteerStatus->act(*this);
            this->addAction(volunteerStatus); 
        } 
        else if (command == "customerStatus") {
            iss >> customerID;
            PrintCustomerStatus* customerStatus = new PrintCustomerStatus(customerID);
            customerStatus->act(*this);
            this->addAction(customerStatus);  
        }
        else if (command == "backup") {
            BackupWareHouse* backUp = new BackupWareHouse();
            backUp->act(*this);
            this->addAction(backUp); 
        }
        else if (command == "restore") {
            RestoreWareHouse* restore = new RestoreWareHouse();
            restore->act(*this);
            this->addAction(restore); 
        }
    }
}

int WareHouse::getOrdersCounter() const
{
    return ordersCounter;
}

int WareHouse::getCustomersCounter() const
{
    return customerCounter;
}

int WareHouse::getVolunteersCounter() const
{
    return volunteerCounter;
}

void WareHouse::addOrder(Order* order)
{
    pendingOrders.push_back(order);
}

void WareHouse::addCustomer(Customer* customer)
{
    customers.push_back(customer);
}

void WareHouse::addAction(BaseAction* action)
{
    actionsLog.push_back(action);
}

Customer &WareHouse::getCustomer(int customerId) const
{
    for (const auto& customer : customers) {
        if (customer->getId() == customerId) {
            return *customer;
        }
    }
    return *defaultCustomer; // the case is no customer
}

Volunteer &WareHouse::getVolunteer(int volunteerId) const 
{
    for (const auto& volunteer : volunteers) {
        if (volunteer->getId() == volunteerId) {
            return *volunteer;
        }
    }
    return *defaultVolunteer; // the case is no volunteer
}

Order &WareHouse::getOrder(int orderId) const
{
    for (const auto& order : pendingOrders) {
        if (order->getId() == orderId) {
            return *order;
        }
    }
    
    for (const auto& order : inProcessOrders) {
        if (order->getId() == orderId) {
            return *order;
        }
    }

    for (const auto& order : completedOrders) {
        if (order->getId() == orderId) {
            return *order;
        }
    }
    
    return *defaultOrder; // the case is no order 
}

const vector<BaseAction*> &WareHouse::getActions() const
{
    return actionsLog;
}

void WareHouse::close() 
{
    isOpen = false;
}

void WareHouse::open() 
{
    isOpen = true;
}


void WareHouse::parsingTxt(const string &configFilePath)
{
    std::ifstream file(configFilePath);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << configFilePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // skip comments or empty lines
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream iss(line);
        string type;
        iss >> type;

        if (type == "customer")
        {
            string name, customerType;
            int distance, maxOrders;
            iss >> name >> customerType >> distance >> maxOrders;

            if (customerType == "soldier")
            {
                Customer *customer = new SoldierCustomer(customerCounter, name, distance, maxOrders);
                customers.push_back(customer);
                customerCounter = customerCounter + 1;
            }

            else if (customerType == "civilian")
            {
                Customer *customer = new CivilianCustomer(customerCounter, name, distance, maxOrders);
                customers.push_back(customer);
                customerCounter = customerCounter + 1;
            }
        }
        else if (type == "volunteer")
        {
            string name, volunteerType;
            int coolDown, maxOrders, maxDistance, distancePerStep;
            

            iss >> name >> volunteerType;

            if (volunteerType == "collector")
            {
                iss >> coolDown;
                Volunteer *volunteer = new CollectorVolunteer(volunteerCounter, name, coolDown);
                volunteers.push_back(volunteer);
                volunteerCounter = volunteerCounter + 1;
            }
            else if (volunteerType == "limited_collector")
            {
                iss >>coolDown>> maxOrders;
                Volunteer *volunteer = new LimitedCollectorVolunteer(volunteerCounter, name, coolDown, maxOrders);
                volunteers.push_back(volunteer);
                volunteerCounter = volunteerCounter + 1;
            }
            else if (volunteerType == "driver")
            {
                iss >> maxDistance >> distancePerStep;
                Volunteer *volunteer = new DriverVolunteer(volunteerCounter, name, maxDistance, distancePerStep);
                volunteers.push_back(volunteer);
                volunteerCounter = volunteerCounter + 1;
            }
            else if (volunteerType == "limited_driver")
            {
                iss >> maxDistance >> distancePerStep >> maxOrders;
                Volunteer *volunteer = new LimitedDriverVolunteer(volunteerCounter, name, maxDistance, distancePerStep, maxOrders);
                volunteers.push_back(volunteer);
                volunteerCounter = volunteerCounter + 1;
            }
        }
    }
    file.close();
}

void WareHouse::increaseCustomersCounter()
{
    customerCounter++;
}

void WareHouse::increaseVolunteersCounter()
{
    volunteerCounter++;
}

void WareHouse::increaseOrdersCounter()
{
    ordersCounter++;
}

vector<Order*> &WareHouse::getPendingOrders()
{
    return pendingOrders;
}
vector<Order*> &WareHouse::getInProcessOrders()
{
    return inProcessOrders;
}
vector<Order*> &WareHouse::getCompletedOrders()
{
    return completedOrders;
}

vector<Volunteer*> &WareHouse::getVolunteers()
{
    return volunteers;
}

vector<Customer*> &WareHouse::getCustomers()
{
    return customers;
}

WareHouse::~WareHouse() {
    delete defaultCustomer;
    delete defaultOrder;
    delete defaultVolunteer;

    for (Customer* customer : customers) 
    {
        if(customer)
            delete customer;
        customer = nullptr;
    }
    customers.clear();
    for (Volunteer* volunteer : volunteers) 
    {
        if(volunteer)
            delete volunteer;
        volunteer = nullptr;
    }
    volunteers.clear();
    for (BaseAction* BaseAction : actionsLog) 
    {
        if(BaseAction)
            delete BaseAction;
        BaseAction = nullptr;
    }
    actionsLog.clear();
    for (Order* completedOrder : completedOrders) 
    {
        if(completedOrder)
            delete completedOrder;
        completedOrder = nullptr;
    }
    completedOrders.clear();
    for (Order* inProcessOrder : inProcessOrders) 
    {
        if(inProcessOrder)
            delete inProcessOrder;
        inProcessOrder = nullptr;
    }
    inProcessOrders.clear();
    for (Order* pendingOrder : pendingOrders) 
    {
        if(pendingOrder)
            delete pendingOrder;
        pendingOrder = nullptr;
    }
    pendingOrders.clear();

}


bool WareHouse::getIsOpen() const
{
    return isOpen;
}

// Copy Constructor
WareHouse::WareHouse(const WareHouse &other) 
:  isOpen(other.getIsOpen()), actionsLog(other.getActions()),volunteers(getVolunteers()), pendingOrders(getPendingOrders()),
inProcessOrders(getInProcessOrders()), completedOrders(getCompletedOrders()), customers(getCustomers()),
customerCounter(other.getCustomersCounter()), volunteerCounter(other.getVolunteersCounter()), ordersCounter(other.getOrdersCounter())


{
    for (auto action : other.actionsLog){actionsLog.push_back(action->clone());}
    for (auto volunteer : other.volunteers){volunteers.push_back(volunteer->clone());}
    for (auto customer : other.customers){customers.push_back(customer->clone());}
    for (auto orderpending : other.pendingOrders){pendingOrders.push_back(orderpending->clone());}
    for (auto inprocess : other.inProcessOrders){inProcessOrders.push_back(inprocess->clone());}
    for (auto completeOrders : other.completedOrders){completedOrders.push_back(completeOrders->clone());}      
}

WareHouse &WareHouse::operator=(const WareHouse &other) // copy assignment operator
{
    if (this == &other) // protect against self-assignment
        return *this;

    delete defaultCustomer;
    delete defaultOrder;
    delete defaultVolunteer;

    for (Customer* customer : customers) 
    {
        if(customer)
            delete customer;
        customer = nullptr;
    }
    customers.clear();
    for (Volunteer* volunteer : volunteers) 
    {
        if(volunteer)
            delete volunteer;
        volunteer = nullptr;
    }
    volunteers.clear();
    for (BaseAction* BaseAction : actionsLog) 
    {
        if(BaseAction)
            delete BaseAction;
        BaseAction = nullptr;
    }
    actionsLog.clear();
    for (Order* completedOrder : completedOrders) 
    {
        if(completedOrder)
            delete completedOrder;
        completedOrder = nullptr;
    }
    completedOrders.clear();
    for (Order* inProcessOrder : inProcessOrders) 
    {
        if(inProcessOrder)
            delete inProcessOrder;
        inProcessOrder = nullptr;
    }
    inProcessOrders.clear();
    for (Order* pendingOrder : pendingOrders) 
    {
        if(pendingOrder)
            delete pendingOrder;
        pendingOrder = nullptr;
    }
    pendingOrders.clear();

    // copy from other
    defaultCustomer = other.defaultCustomer;
    defaultVolunteer = other.defaultVolunteer;
    defaultOrder = other.defaultOrder;
    isOpen = other.getIsOpen();
    customerCounter = other.getCustomersCounter();
    volunteerCounter = other.getVolunteersCounter();
    ordersCounter = other.getOrdersCounter();


    for (auto action : other.actionsLog){actionsLog.push_back(action->clone());}
    for (auto orderpending : other.pendingOrders){pendingOrders.push_back(orderpending->clone());}
    for (auto inprocess : other.inProcessOrders){inProcessOrders.push_back(inprocess->clone());}
    for (auto completeOrders : other.completedOrders){completedOrders.push_back(completeOrders->clone());}
    for (auto customer : other.customers){customers.push_back(customer->clone());}
    for (auto volunteer : other.volunteers){volunteers.push_back(volunteer->clone());}

    return *this;
}


// Move constructor
WareHouse::WareHouse(WareHouse &&other) noexcept : isOpen(other.getIsOpen()), customerCounter(other.getCustomersCounter()), volunteerCounter(other.getVolunteersCounter()), ordersCounter(other.getOrdersCounter())
{
    actionsLog = std::move(other.actionsLog);
    pendingOrders = std::move(other.pendingOrders);
    inProcessOrders = std::move(other.inProcessOrders);
    completedOrders = std::move(other.completedOrders);
    customers = std::move(other.customers);
    volunteers = std::move(other.volunteers);

    // Set the moved-from object's pointers to nullptr
    other.actionsLog.clear();
    other.pendingOrders.clear();
    other.inProcessOrders.clear();
    other.completedOrders.clear();
    other.customers.clear();
    other.volunteers.clear();
}

// Move assign operator
WareHouse& WareHouse::operator=(WareHouse&& other) noexcept
{
    if (this != &other)
    {
        // delete current resources
        this->~WareHouse();
        actionsLog.clear();
        pendingOrders.clear();
        inProcessOrders.clear();
        completedOrders.clear();
        customers.clear();
        volunteers.clear();

        // Transfer the ownership of resources from 'other'
        isOpen = other.isOpen;
        customerCounter = other.customerCounter;
        volunteerCounter = other.volunteerCounter;
        ordersCounter = other.ordersCounter;
        actionsLog = std::move(other.actionsLog);
        pendingOrders = std::move(other.pendingOrders);
        inProcessOrders = std::move(other.inProcessOrders);
        completedOrders = std::move(other.completedOrders);
        customers = std::move(other.customers);
        volunteers = std::move(other.volunteers);

    }

    return *this;
}

////////////////////////////////////////////////////////////////////////

void WareHouse::moveOrderToInProcess(Order *order)
{
    if (order->getStatus() == OrderStatus::PENDING)
    {
        order->setStatus(OrderStatus::COLLECTING);
    }
    else if (order->getStatus() == OrderStatus::COLLECTING)
    {
        order->setStatus(OrderStatus::DELIVERING);
    }
    inProcessOrders.push_back(order);
}

void WareHouse::removeDupInPending()
{
    for (Order *order : inProcessOrders)
    {
        for (Order *order2 : pendingOrders)
        {
            if (order->getId() == order2->getId())
            {
                pendingOrders.erase(remove(pendingOrders.begin(), pendingOrders.end(), order), pendingOrders.end());
            }
        }
    }
}

void WareHouse::moveOrderToCompleted(Order *order)
{
    order->setStatus(OrderStatus::COMPLETED);
    completedOrders.push_back(order);
}


void WareHouse::removeDupCompleted()
{
    for (Order *order : completedOrders)
    {
        for (Order *order2 : inProcessOrders)
        {
            if (order->getId() == order2->getId())
            {
                inProcessOrders.erase(remove(inProcessOrders.begin(), inProcessOrders.end(), order), inProcessOrders.end());
            }
        }
    }
}

void WareHouse::moveOrderToInPending(Order *order)
{
    pendingOrders.push_back(order);
}

void WareHouse::removeDupInProcess()
{
    for (Order *order : pendingOrders)
    {
        for (Order *order2 : inProcessOrders)
        {
            if (order->getId() == order2->getId())
            {
                inProcessOrders.erase(remove(inProcessOrders.begin(), inProcessOrders.end(), order), inProcessOrders.end());
            }
        }
    }
}

bool WareHouse::checkForValidVolunteer(int volunteerId)
{
    for (Volunteer *volunteer : volunteers)
    {
        if (volunteer->getId() == volunteerId)
        {
            return true;
        }
    }
    return false;
}

void WareHouse::simulateStep(int numOfSteps, BaseAction *action)
{
    for (int i = 0; i < numOfSteps; i++)
    {
        for (Order *order : pendingOrders)
        {
            for (Volunteer *volenteer : getVolunteers())
            {
                if (volenteer->canTakeOrder(*order))
                {
                    volenteer->acceptOrder(*order);
                    if (order->getStatus() == OrderStatus::PENDING)
                    {
                        order->setCollectorId(volenteer->getId());
                    }
                    else if (order->getStatus() == OrderStatus::COLLECTING)
                    {
                        order->setDriverId(volenteer->getId());
                    }
                    moveOrderToInProcess(order);
                    break;
                }
            }
        }
        removeDupInPending();

        for (Volunteer *volenteer : getVolunteers())
        {
            volenteer->step();
            int orderId = volenteer->getCompletedOrderId();
            if (!volenteer->isBusy() && orderId != -1)
            {
                Order *order = &getOrder(orderId);
                if (order->getStatus() == OrderStatus::COLLECTING)
                {
                    moveOrderToInPending(order);
                    removeDupInProcess();
                }
                else if (order->getStatus() == OrderStatus::DELIVERING)
                {
                    moveOrderToCompleted(order);
                    removeDupCompleted();
                }
            }
        }
        auto condition = [](Volunteer *volunteer)
        { return !volunteer->hasOrdersLeft() && !volunteer->isBusy(); };

        for (auto it = volunteers.begin(); it != volunteers.end();)
        {
            if (condition(*it))
            {
                delete *it;
                it = volunteers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}