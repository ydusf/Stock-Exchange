#pragma once

#include "SystemMediator.hxx"

template<typename TStrategy> 
class TradingStrategy 
{ 
protected:
	SystemMediator& m_systemMediator;
	
public:
	TradingStrategy(SystemMediator& systemMediator)
		: m_systemMediator(systemMediator)
	{};

	void Run()
	{
		static_cast<TStrategy*>(this)->RunImpl();
	}

	~TradingStrategy()
	{};
};


class TradingStrategySimple : public TradingStrategy<TradingStrategySimple>
{
private:
	Account& m_account;

	double m_buyPrice = 100;
	double m_sellPrice = 120;
	std::string m_ticker;
	
public:
	TradingStrategySimple(SystemMediator& systemMediator, Account& account, std::string ticker);

	void RunImpl();

	~TradingStrategySimple();

private:
	void Execute();

};