#include <iostream>
#include <string>
#include <memory>

using namespace std;

enum class FurnitureType {
    Modern,
    Classic,
    Luxury
};

class Ichair {
public:
    virtual void Size() = 0;
    virtual ~Ichair() = default;
};

class IFan {
public:
    virtual void Speed() = 0;
    virtual ~IFan() = default;
};

class ModernChair : public Ichair {
public:
    void Size() override {
        cout << "Modern Chair Size is 5 feet" << endl;
    }
};

class ClassicChair : public Ichair {
public:
    void Size() override {
        cout << "Classic Chair Size is 4 feet" << endl;
    }
};

class LuxuryChair : public Ichair {
public:
    void Size() override {
        cout << "Luxury Chair Size is 6 feet" << endl;
    }
};

class ModernFan : public IFan {
public:
    void Speed() override {
        cout << "Modern Fan Speed is 3" << endl;
    }
};

class ClassicFan : public IFan {
public:
    void Speed() override {
        cout << "Classic Fan Speed is 2" << endl;
    }
};

class LuxuryFan : public IFan {
public:
    void Speed() override {
        cout << "Luxury Fan Speed is 4" << endl;
    }
};

class IAbstractFactory {
public:
    virtual std::unique_ptr<Ichair> CreateChair() = 0;
    virtual std::unique_ptr<IFan> CreateFan() = 0;
    virtual ~IAbstractFactory() = default;
};


class ModernFactory : public IAbstractFactory {
public:
    std::unique_ptr<Ichair> CreateChair() override {
        return std::make_unique<ModernChair>();
    }
    std::unique_ptr<IFan> CreateFan() override {
        return std::make_unique<ModernFan>();
    }
};

class ClassicFactory : public IAbstractFactory {
public:
    std::unique_ptr<Ichair> CreateChair() override {
        return std::make_unique<ClassicChair>();
    }
    std::unique_ptr<IFan> CreateFan() override {
        return std::make_unique<ClassicFan>();
    }
};

class LuxuryFactory : public IAbstractFactory {
public:
    std::unique_ptr<Ichair> CreateChair() override {    
        return std::make_unique<LuxuryChair>();
    }
    std::unique_ptr<IFan> CreateFan() override {
        return std::make_unique<LuxuryFan>();
    }
};

class FactoryProducer {
public:
    static std::unique_ptr<IAbstractFactory> GetFactory(FurnitureType type) {
        switch (type) {
            case FurnitureType::Modern:
                return std::make_unique<ModernFactory>();
            case FurnitureType::Classic:
                return std::make_unique<ClassicFactory>();
            case FurnitureType::Luxury:
                return std::make_unique<LuxuryFactory>();
            default:
                return nullptr;
        }
    }
};

class Client {

public:
    Client(std::unique_ptr<IAbstractFactory> factory) : factory_(std::move(factory)) {}
    void CreateFurniture(FurnitureType type) {
        factory_ = FactoryProducer::GetFactory(type);
        auto chair = factory_->CreateChair();
        auto fan = factory_->CreateFan();

        chair->Size();
        fan->Speed();
    }
private:
    std::unique_ptr<IAbstractFactory> factory_;
};

int main() {

    Client client(FactoryProducer::GetFactory(FurnitureType::Modern));
    client.CreateFurniture(FurnitureType::Modern);

    client.CreateFurniture(FurnitureType::Classic);

    client.CreateFurniture(FurnitureType::Luxury);

    return 0;
}