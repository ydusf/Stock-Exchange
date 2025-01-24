#include "TradingStrategy.hxx"

TradingStrategySimple::TradingStrategySimple(Exchange& exchange, Account& account, std::string ticker)
	: TradingStrategy(exchange), m_account(account), m_ticker(ticker)
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