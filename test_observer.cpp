#include <memory>
#include <mutex>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

struct Member
{
	Member(int id, std::string name) : m_id(id), m_name(name) {}
	int m_id;
	std::string m_name;
};

struct Institution
{
	Institution(int id, std::string name, std::string addr) : m_id(id), m_name(name), m_addr(addr) {}
	int m_id{ 0 };
	std::string m_name{ "" };
	std::string m_addr{ "" };
};


class Observer
{
public:
	void SetMemberData(int id, const std::vector<Member>& data, const std::string info) 
	{
		std::cout << "SetMemberData..." << "id:" << id << " info:" << info.c_str() << std::endl;
		for (auto& it : data) 
		{
			std::cout << "mem_id:" << it.m_id << " mem_name:" << it.m_name << std::endl;
		}
	}

	void SetInsData(int id, const Institution info) 
	{
		std::cout << "SetInsData..." << "id:" << id << std::endl;
		std::cout << "ins_id:" << info.m_id << " ins_name: " << info.m_name << " ins_addr:" << info.m_addr << std::endl;

	}
};

class IDataIf
{
public:
	virtual void OutputMemberData(int id, const std::vector<Member>& data, const std::string info) = 0;
	virtual void OutputInsData(int id, const Institution info) = 0;
	virtual ~IDataIf() {};
};

class NotifyServiceImpl
{
public:
	NotifyServiceImpl() = default;
	virtual ~NotifyServiceImpl() {}

	void CreateContext(std::shared_ptr<Observer> obs)
	{
		std::lock_guard<std::mutex> lck(m_mt);
		m_obsContainer.emplace_back(std::weak_ptr<Observer>(obs));
	}


	void DeleteContext(std::shared_ptr<Observer> obs)
	{
		std::lock_guard<std::mutex> lck(m_mt);
		m_obsContainer.erase(std::remove_if(std::begin(m_obsContainer), std::end(m_obsContainer), [&](const std::weak_ptr<Observer> &it) {
			return !it.expired() && it.lock() == obs; }), std::end(m_obsContainer));
	}

protected:

	void ClearExpiredObs()
	{
		m_obsContainer.erase(std::remove_if(std::begin(m_obsContainer), std::end(m_obsContainer), [&](const std::weak_ptr<Observer>& it) {
			return it.expired(); }), std::end(m_obsContainer));
	}

protected:
	std::list<std::weak_ptr<Observer>> m_obsContainer;
	std::mutex	 m_mt;
};

class DataServiceImpl final : public NotifyServiceImpl, public IDataIf
{
public:
	DataServiceImpl() : NotifyServiceImpl() {}
	~DataServiceImpl() {}

	void OutputMemberData(int id, const std::vector<Member>& data, const std::string info) override
	{
		std::lock_guard<std::mutex> lck(m_mt);
		for (auto& it : m_obsContainer) 
		{
			std::shared_ptr<Observer> obs = it.lock();
			if (obs != nullptr)
			{ 
				obs->SetMemberData(id, data, info); 
			}
		}
		ClearExpiredObs();
	}

	void OutputInsData(int id, const Institution info) override
	{
		std::lock_guard<std::mutex> lck(m_mt);
		for (auto& it : m_obsContainer) 
		{
			std::shared_ptr<Observer> obs = it.lock();
			if (obs != nullptr)
			{ 
				obs->SetInsData(id, info); 
			}
		}
		ClearExpiredObs();
	}

};


int main() {

	std::shared_ptr<Observer> obs1 = std::make_shared<Observer>();
	std::shared_ptr<Observer> obs2 = std::make_shared<Observer>();
	DataServiceImpl server;
	server.CreateContext(obs1);
	server.CreateContext(obs2);

	IDataIf& dataHandler(server);
	std::vector<Member> data{ {1, "john"}, {2,"david"} };
	dataHandler.OutputMemberData(1, data, "member");

	Institution ins{ 1001, "Institution", "addresss" };
	dataHandler.OutputInsData(1001, ins);

	server.DeleteContext(obs1);
	server.DeleteContext(obs2);

	system("pause");

	return 0;
}






