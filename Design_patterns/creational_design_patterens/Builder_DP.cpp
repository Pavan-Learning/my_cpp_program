#include <iostream>

/**
 * Advantages of Builder Design Pattern:
 * This is creational design pattern example of Builder Design Pattern.
 * Builder Design Pattern Example.
 * this is allowing us to create complex objects steps by steps into smaller objects and then finally assemble them into a final object.
 * this allow us avoid constructs with many parameters and improve readability of the code.
 * supports step by step construction of complex objects.
 * Makes optional parameters easy to manage.
 * Reuses the same construction process for different products.
 * Easier to maintain and extend.
 * Can enforce object validation before creation.
 * 
 * 
 * Disadvantages of Builder Design Pattern:
 * More code to write.
 * Increased complexity due to additional classes and interfaces.
 */


 // this is final product class we at end to client
/* class Computer {
    public:
        std::string CPU;
        std::string GPU;
        int RAM;
        int storage;
};

class Builder {
    public:
        virtual void setCPU() = 0;
        virtual void setGPU() = 0;
        virtual void setRAM() = 0;
        virtual void setStorage() = 0;
        virtual Computer getComputer() = 0;
};

class GamerComputerBuilder : public Builder {
    private:
        Computer computer;
    public:        
        void setCPU() override {
            computer.CPU = "Intel i9"; // Set CPU for the computer
        }
        
        void setGPU() override {
            computer.GPU = "NVIDIA RTX 4090"; // Set GPU for the computer
        }
        
        void setRAM() override {
            computer.RAM = 16; // Set RAM for the computer
        }
        
        void setStorage() override {
            computer.storage = 512; // Set storage for the computer
        }
        
        Computer getComputer() override {
            return computer;
        }
};


// this director class is resposiable to tell the order of steps to build the final product
class Director {
    public:
        
        void constructComputer(Builder& builder) {
            builder.setCPU();
            builder.setGPU();
            builder.setRAM();
            builder.setStorage();
        }
};

int main() {
    GamerComputerBuilder gamerBuilder;
    Director director;
    
    director.constructComputer(gamerBuilder);
    
    Computer gamerComputer = gamerBuilder.getComputer();

    std::cout << "Gamer Computer Specifications:" << std::endl;
    std::cout << "CPU: " << gamerComputer.CPU << std::endl;
    std::cout << "GPU: " << gamerComputer.GPU << std::endl;
    std::cout << "RAM: " << gamerComputer.RAM << " GB" << std::endl;
    std::cout << "Storage: " << gamerComputer.storage << " GB" << std::endl;

    return 0;
}
*/

// the below program is another way to implement the builder design pattern without using the director class. 
// In this approach, we can directly use the builder to construct the computer in a more flexible way.

/* In the above Director class is optional we can directly use the builder to construct the computer in new way */

class Computer{
    private:
        std::string CPU;
        std::string GPU;
        int RAM;
        int storage;
    public:
        void Show() const
    {
        std::cout << "\nComputer Configuration\n";
        std::cout << "CPU      : " << CPU << std::endl;
        std::cout << "RAM      : " << RAM << std::endl;
        std::cout << "Storage  : " << storage << std::endl;
        std::cout << "GPU      : " << GPU << std::endl;
    }
    
    public:
        class Builder{
            private:
                std::string cpu;
                std::string gpu;
                int ram = 0;
                int storage = 0;
            public:
                Builder& setCPU(const std::string& cpu) {
                    this->cpu = cpu;
                    return *this;
                }
                Builder& setGPU(const std::string& gpu) {
                    this->gpu = gpu;
                    return *this;
                }
                Builder& setRAM(int ram) {
                    this->ram = ram;
                    return *this;
                }
                Builder& setStorage(int storage) {
                    this->storage = storage;
                    return *this;
                }
                Computer build() {
                    Computer c;
                    c.CPU = cpu;
                    c.GPU = gpu;
                    c.RAM = ram;
                    c.storage = storage;
                    return c;
                }
        };
};

int main() {
    Computer gamerComputer = Computer::Builder()
        .setCPU("Intel i9")
        .setGPU("NVIDIA RTX 4090")
        .setRAM(16)
        .setStorage(512)
        .build();

    gamerComputer.Show();

    return 0;
}
