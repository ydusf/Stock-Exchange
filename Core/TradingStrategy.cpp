#include "TradingStrategy.hxx"

TradingStrategySimple::TradingStrategySimple(SystemMediator& systemMediator, Account& account, std::string ticker)
	: TradingStrategy(systemMediator), m_account(account), m_ticker(ticker)
{};

TradingStrategySimple::~TradingStrategySimple()
{

};

void TradingStrategySimple::RunImpl()
{

}

void TradingStrategySimple::Execute()
{
	
}