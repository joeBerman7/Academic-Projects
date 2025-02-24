#pragma once
#include <string>
#include <vector>

#include "../include/Order.h"
#include "../include/Customer.h"

class BaseAction;
class Volunteer;

// Warehouse responsible for Volunteers, Customers Actions, and Orders.

class WareHouse {

    public:
        WareHouse(const string &configFilePath);
        //rule of 5:
        WareHouse(const WareHouse &other);
        WareHouse& operator=(const WareHouse &other);
        ~WareHouse();
        WareHouse(WareHouse&& other) noexcept;
        WareHouse& operator=(WareHouse&& other) noexcept;
        //
        void start();
        void addOrder(Order* order);
        void addCustomer(Customer* customer); // our added function
        void addAction(BaseAction* action);
        Customer &getCustomer(int customerId) const;
        Volunteer &getVolunteer(int volunteerId) const;
        Order &getOrder(int orderId) const;
        const vector<BaseAction*> &getActions() const;
        void close();
        void open();
        
        // our func
        bool getIsOpen() const;
        void parsingTxt(const string &configFilePath);
        int getCustomersCounter() const;
        void increaseCustomersCounter();
        int getVolunteersCounter() const;
        void increaseVolunteersCounter();
        int getOrdersCounter() const;
        void increaseOrdersCounter();
        vector<Order*> &getPendingOrders();
        vector<Order*> &getInProcessOrders();
        vector<Order*> &getCompletedOrders();
        vector<Volunteer*> &getVolunteers();
        vector<Customer*> &getCustomers();   
        void moveOrderToInProcess(Order *order);
        void removeDupInPending();
        void moveOrderToCompleted(Order *order);
        void removeDupCompleted();
        void moveOrderToInPending(Order *order);
        void removeDupInProcess();
        bool checkForValidVolunteer(int volunteerId);
        void simulateStep(int numOfSteps, BaseAction *action);

    private:
        bool isOpen;
        vector<BaseAction*> actionsLog;
        vector<Volunteer*> volunteers;
        vector<Order*> pendingOrders;
        vector<Order*> inProcessOrders;
        vector<Order*> completedOrders;
        vector<Customer*> customers;
        int customerCounter; //For assigning unique customer IDs
        int volunteerCounter; //For assigning unique volunteer IDs   
        int ordersCounter;
        Customer* defaultCustomer;
        Volunteer* defaultVolunteer;
        Order* defaultOrder;
};

