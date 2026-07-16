#include <iostream>

class Car{
    public:
        Car(){
            std::cout << "******** Base class :: Car constructor*******" << std::endl;
        }
        virtual void start()
        {
            std::cout << " Base class :: Car is starting" << std::endl;
        }
        virtual void stop()
        {
            std::cout << " Base class :: Car is stopping" << std::endl;
        }
        virtual void accelerate()
        {
            std::cout << " Base class :: Car is accelerating" << std::endl;
        }
        virtual ~Car(){
            std::cout << " ***********Base class :: Car destructor*******" << std::endl;
        }
};

class Tata : public Car{
    public:
        Tata(){
            std::cout << " ******** Tata class :: Tata constructor *******" << std::endl;
        }
        void start() override
        {
            std::cout << " Tata class :: Tata is starting" << std::endl;
        }
        void accelerate() override
        {
            std::cout << " Tata class :: Tata is accelerating" << std::endl;
        }

        virtual void newEngine()
        {
            std::cout << " Tata class :: Tata has new engine" << std::endl;
        }
        virtual void tatagroup()
        {
            std::cout << " Tata class :: Tata has new group" << std::endl;
        }
        virtual ~Tata(){
            std::cout << " ******** Tata class :: Tata destructor *******" << std::endl;
        }
};

class Maruti : public Tata{
    public:
        Maruti(){
            std::cout << " ******** Maruti class :: Maruti constructor *******" << std::endl;
        }
        virtual ~Maruti(){
            std::cout << " ******** Maruti class :: Maruti destructor *******" << std::endl;
        }
        void start() override
        {
            std::cout << " Maruti class :: Maruti is starting" << std::endl;
        }
        void stop() override
        {
            std::cout << " Maruti class :: Maruti is stopping" << std::endl;
        }
        void accelerate() override
        {
            std::cout << " Maruti class :: Maruti is accelerating" << std::endl;
        }
        void newEngine() override
        {
            std::cout << " Maruti class :: Maruti has new engine" << std::endl;
        }

        virtual void garbox(){
            std::cout << " Maruti class :: Maruti has new garbox" << std::endl;
        }
};

class Honda : public Maruti{
    public:
        Honda(){
            std::cout << " ******** Honda class :: Honda constructor *******" << std::endl;
        }
        virtual ~Honda(){
            std::cout << " ******** Honda class :: Honda destructor *******" << std::endl;
        }
        void start() override
        {
            std::cout << " Honda class :: Honda is starting" << std::endl;
        }
        void stop() override
        {
            std::cout << " Honda class :: Honda is stopping" << std::endl;
        }
        void newEngine() override
        {
            std::cout << " Honda class :: Honda has new engine" << std::endl;
        }

        void garbox() override{
            std::cout << " Honda class :: Honda has new garbox" << std::endl;
        }

};

class Suzuki {
    public:
        Suzuki(){
            std::cout << " ******** Suzuki class :: Suzuki constructor *******" << std::endl;
        }
        virtual ~Suzuki(){
            std::cout << " ******** Suzuki class :: Suzuki destructor *******" << std::endl;
        }
        virtual void start()
        {
            std::cout << " Suzuki class :: Suzuki is starting" << std::endl;
        }
        virtual void stop()
        {
            std::cout << " Suzuki class :: Suzuki is stopping" << std::endl;
        }
        virtual void accelerate()
        {
            std::cout << " Suzuki class :: Suzuki is accelerating" << std::endl;
        }

};

class Hyundai : public Suzuki, public Car{
    public:
        Hyundai(){
            std::cout << " ******** Hyundai class :: Hyundai constructor *******" << std::endl;
        }
        virtual ~Hyundai(){
            std::cout << " ******** Hyundai class :: Hyundai destructor *******" << std::endl;
        }
        void start() override
        {
            std::cout << " Hyundai class :: Hyundai is starting" << std::endl;
        }

};

int main()
{

    // Here we are creating Honda class object with base class pointer so this pointer 
    // will create vptr for Honda class and it will point to the vtable of Honda class 
    // so when we call the methods using base class pointer it will call the derived class
    // methods if they are overridden in derived class otherwise it will call the base class methods.
    Car* car = new Honda();
    car->start();
    car->stop();
    // car->garbox(); // here it will give error due to base class doesnt have gearbox() method 
                     //  here base class pointer is pointing to derived class object so it can only access the base class methods   
    car->accelerate();

    std::cout << "********** Now creating Honda class object with derived class pointer **********" << std::endl;

    Honda* honda = new Honda();
    honda->garbox();
    honda->newEngine();
    honda->start();
    honda->stop();
    honda->accelerate();
    honda->tatagroup();

    std::cout << "--------------- Now creating Hyundai class object with derived class pointer ------------" << std::endl;

    Hyundai* hyundai = new Hyundai();
    hyundai->start();
    // hyundai->accelerate(); //here acceleratec () method is present in both base classes
                           // so it will give error due to ambiguity because compiler doesn't
                           // know which base class method to call so we need to resolve this ambiguity by using scope resolution operator

    
    delete hyundai;
    delete honda;
    delete car;
}   