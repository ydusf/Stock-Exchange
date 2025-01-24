#pragma once

#include "Exchange.hxx"

template<typename TStrategy> 
class TradingStrategy 
{ 
protected:
	Exchange& m_exchange;
	
public:
	TradingStrategy(Exchange& exchange)
		: m_exchange(exchange)
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
	TradingStrategySimple(Exchange& exchange, Account& account, std::string ticker);

	void RunImpl();

	~TradingStrategySimple();

private:
	void Execute();

};