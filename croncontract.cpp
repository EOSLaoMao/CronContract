#include <eosiolib/currency.hpp>
#include <eosiolib/transaction.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>
#include "croncontract.hpp"

using namespace eosio;
using namespace eosiosystem;
using namespace cron; 
using std::string;

class croncontract : contract {

private:

    void schedule_cron(account_name account,
                    std::string cronaction)
    {
        print("schedule_cron called");
        eosio::transaction out;

        action act = action(
          permission_level{ account, N(cronperm) },
          account, string_to_name(cronaction.c_str()),
          std::make_tuple()
        );

        out.actions.emplace_back(act);
        out.send((uint128_t(code_account) << 64) | current_time(), code_account, true);
    }

public:
    using contract::contract;
    croncontract( name self ) : contract(self){}

    // @abi action addcronjob
    void addcronjob(account_name account,
                    std::string cronaction,
                    uint64_t interval)
    {
        require_auth(account);

        schedule_cron(account, cronaction);

        cronjob_table c(code_account, code_account);
        auto itr = c.find(account);
        if(itr == c.end()) {
          print(" | emplace");
          c.emplace(ram_payer, [&](auto &i) {
            i.account = account;
            i.action = cronaction;
            i.interval = interval;
            i.version = 0;
            i.num_executions = 0;
            i.created_at = now();
            i.updated_at = now();
          });
        } else {
          print(" | modify");
          c.modify(itr, ram_payer, [&](auto &i) {
            i.action = cronaction;
            i.interval = interval;
            i.updated_at = now();
          });
        }
    }

    // @abi action schedule
    void schedule(account_name account, std::string action){
    }

    void apply( account_name contract, account_name action ) {
      if( contract != _self ) return;
      auto& thiscontract = *this;
      switch( action ) {
        EOSIO_API( croncontract,
                (schedule)
                (addcronjob) )
      };
    }
};

extern "C" {
  [[noreturn]] void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
    croncontract c( receiver );
    c.apply( code, action );
    eosio_exit(0);
  }
}
