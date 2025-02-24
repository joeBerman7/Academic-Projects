#include <string>
#include <vector>
#include <sstream>
#include "../include/Order.h"
#include "../include/Volunteer.h"

#define NO_ORDER -1

using std::string;
using std::vector;

Volunteer::Volunteer(int id, const string &name) :
    completedOrderId(NO_ORDER), activeOrderId(NO_ORDER), id(id), name(name) {}

Volunteer::~Volunteer() {}

int Volunteer::getId() const 
{
    return id;
}

const string &Volunteer::getName() const 
{
    return name;
}


int Volunteer::getActiveOrderId() const 
{
    return activeOrderId;
}

int Volunteer::getCompletedOrderId() const 
{
    return completedOrderId;
}

bool Volunteer::isBusy() const 
{
    return activeOrderId != NO_ORDER;
}


// CollectorVolunteer class implementation

CollectorVolunteer::CollectorVolunteer(int id, const string &name, int coolDown): 
    Volunteer(id, name) ,coolDown(coolDown), timeLeft(0)
    {
        completedOrderId = Volunteer::completedOrderId;
        activeOrderId = Volunteer::activeOrderId;
    }

CollectorVolunteer *CollectorVolunteer::clone() const
{
    return new CollectorVolunteer(*this);
}

void CollectorVolunteer::step(){
    if(timeLeft >0){
        timeLeft--;
        if(timeLeft == 0){
            completedOrderId = activeOrderId;
            activeOrderId = NO_ORDER;
        }
    }
}

int CollectorVolunteer::getCoolDown() const
{
    return coolDown;
}

int CollectorVolunteer::getTimeLeft() const
{
    return timeLeft;
}

bool CollectorVolunteer::decreaseCoolDown()
{
    if(timeLeft > 0){
        timeLeft--;
        return timeLeft == 0;
    }
    return false;
}

bool CollectorVolunteer::hasOrdersLeft() const{
    return true;
}

bool CollectorVolunteer::canTakeOrder(const Order &order) const
{
    return !isBusy() && order.getStatus() == OrderStatus::PENDING;
}

void CollectorVolunteer::acceptOrder(const Order &order) 
{
    if(canTakeOrder(order)){
        activeOrderId = order.getId();
        completedOrderId = NO_ORDER; 
        timeLeft = coolDown;
    }
}

string CollectorVolunteer::toString() const 
{
    return "VolunteerID:" + std::to_string(getId()) + "\n" +
            "isBusy:" + std::to_string(isBusy()) + "\n" +
            "OrderId:" + std::to_string(getActiveOrderId()) + "\n" +
            "timeLeft:" + std::to_string(timeLeft) + "\n" +
            "ordersLeft: No Limit";
}

// LimitedCollectorVolunteer class implementation

LimitedCollectorVolunteer::LimitedCollectorVolunteer(int id, const string &name, int coolDown ,int maxOrders)
    :CollectorVolunteer(id, name, coolDown), maxOrders(maxOrders), ordersLeft(maxOrders) 
    {
        completedOrderId = CollectorVolunteer::getCompletedOrderId();
        activeOrderId = CollectorVolunteer::getActiveOrderId();
    }


LimitedCollectorVolunteer *LimitedCollectorVolunteer::clone() const
{
    return new LimitedCollectorVolunteer(*this);
}

bool LimitedCollectorVolunteer::hasOrdersLeft() const
{
    return ordersLeft > 0;
}

bool LimitedCollectorVolunteer::canTakeOrder(const Order &order) const
{

    return CollectorVolunteer::canTakeOrder(order) && hasOrdersLeft();
}

void LimitedCollectorVolunteer::acceptOrder(const Order &order)
{
    CollectorVolunteer::acceptOrder(order);
    ordersLeft--;
}

int LimitedCollectorVolunteer::getMaxOrders() const
{
    return maxOrders;
}

int LimitedCollectorVolunteer::getNumOrdersLeft() const
{
    return ordersLeft;
}

string LimitedCollectorVolunteer::toString() const
{
        return "VolunteerID:" + std::to_string(getId()) + "\n" +
            "isBusy:" + std::to_string(isBusy()) + "\n" +
            "OrderId:" + std::to_string(getActiveOrderId()) + "\n" +
            "timeLeft:" + std::to_string(getTimeLeft()) + "\n" +
            "ordersLeft:" + std::to_string(ordersLeft);
}


// DriverVolunteer class implementation

DriverVolunteer::DriverVolunteer(int id, const string &name, int maxDistance, int distancePerStep) :
    Volunteer(id, name), maxDistance(maxDistance), distancePerStep(distancePerStep), distanceLeft(0) {}

DriverVolunteer *DriverVolunteer::clone() const
{
    return new DriverVolunteer(*this);
}

int DriverVolunteer::getDistanceLeft() const
{
    return distanceLeft;
}

int DriverVolunteer::getMaxDistance() const
{
    return maxDistance;
}

int DriverVolunteer::getDistancePerStep() const
{
    return distancePerStep;
}

bool DriverVolunteer::decreaseDistanceLeft()
{
    distanceLeft = distanceLeft - distancePerStep;
    if (distanceLeft <0)
    {
        distanceLeft =0;
        return true;
    }
    return false;
}

bool DriverVolunteer::hasOrdersLeft() const
{
    return true;
}

bool DriverVolunteer::canTakeOrder(const Order &order) const
{
    return !isBusy() && order.getDistance() <= maxDistance && order.getStatus() == OrderStatus::COLLECTING;
}

void DriverVolunteer::acceptOrder(const Order &order)
{
    if(canTakeOrder(order)) {
        activeOrderId = order.getId();
        completedOrderId = NO_ORDER;
        distanceLeft = order.getDistance();
    }
}

void DriverVolunteer::step() {
    if(distanceLeft > 0){
        distanceLeft = distanceLeft - distancePerStep;
        if(distanceLeft <=0){
            completedOrderId = activeOrderId;
            activeOrderId = NO_ORDER;
        }
    }
}

string DriverVolunteer::toString() const {
    return "VolunteerID:" + std::to_string(getId()) + "\n" +
        "isBusy:" + std::to_string(isBusy()) + "\n" +
        "OrderId:" + std::to_string(getActiveOrderId()) + "\n" +
        "distanceLeft:" + std::to_string(distanceLeft) + "\n" +
        "ordersLeft: No Limit";
}


// LimitedDriverVolunteer class implementation

LimitedDriverVolunteer::LimitedDriverVolunteer(int id, const string &name, int maxDistance, int distancePerStep,int maxOrders) 
    : DriverVolunteer(id, name, maxDistance, distancePerStep), maxOrders(maxOrders), ordersLeft(maxOrders){}

LimitedDriverVolunteer *LimitedDriverVolunteer::clone() const
{
    return new LimitedDriverVolunteer(*this);
}

int LimitedDriverVolunteer::getMaxOrders() const
{
    return maxOrders;
}

int LimitedDriverVolunteer::getNumOrdersLeft() const
{
    return ordersLeft;
}

bool LimitedDriverVolunteer::hasOrdersLeft() const
{
    return ordersLeft > 0;
}

bool LimitedDriverVolunteer::canTakeOrder(const Order &order) const
{
    return DriverVolunteer::canTakeOrder(order) && hasOrdersLeft();
}

void LimitedDriverVolunteer::acceptOrder(const Order &order)
{
    DriverVolunteer::acceptOrder(order);
    ordersLeft--;
}

string LimitedDriverVolunteer::toString() const
{
    return "VolunteerID:" + std::to_string(getId()) + "\n" +
        "isBusy:" + std::to_string(isBusy()) + "\n" +
        "OrderId:" + std::to_string(getActiveOrderId()) + "\n" +
        "distanceLeft:" + std::to_string(getDistanceLeft()) + "\n" +
        "ordersLeft:" + std::to_string(ordersLeft);
}