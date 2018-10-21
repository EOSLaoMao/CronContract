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

    void call_cron(account_name account,
                    std::string cronaction,
                    uint64_t nonce)
    {
        eosio::transaction out;

        action act = action(
          permission_level{ account, N(cronperm) },
          account, string_to_name(cronaction.c_str()),
          std::make_tuple()
        );

        out.actions.emplace_back(act);
        out.send((uint128_t(code_account) << 64) | current_time() | nonce, code_account, true);
    }

    void call_next(account_name account, uint64_t interval, uint64_t version, uint64_t nonce)
    {
        eosio::transaction out;
        action act = action(
          permission_level{ code_account, N(active) },
          code_account, N(schedule),
          std::make_tuple(account, version)
        );
        out.actions.emplace_back(act);
        out.delay_sec = interval;
        out.send((uint128_t(code_account) << 64) | current_time() | nonce, code_account, true);
    }

public:
    using contract::contract;
    croncontract( name self ) : contract(self){}

    // @abi action addcronjob
    void addcronjob(account_name account,
                    std::string cronaction,
                    uint64_t interval,
                    uint64_t version)
    {
        require_auth(account);

        call_cron(account, cronaction, current_time());

        cronjob_table c(code_account, code_account);
        auto itr = c.find(account);
        if(itr == c.end()) {
          c.emplace(ram_payer, [&](auto &i) {
            i.account = account;
            i.action = cronaction;
            i.interval = interval;
            i.is_active = 1;
            i.version = 0;
            i.num_executions = 0;
            i.created_at = now();
            i.updated_at = now();
          });
        } else {
          c.modify(itr, ram_payer, [&](auto &i) {
            i.is_active = 1;
            i.action = cronaction;
            i.interval = interval;
            i.version = version;
            i.updated_at = now();
          });
        }
    }

    // @abi action schedule
    void schedule(account_name account, uint64_t version){
        cronjob_table c(code_account, code_account);
        auto itr = c.find(account);
        eosio_assert(itr != c.end(), "cronjob not found!");
        eosio_assert(itr->is_active == 1, "cronjob is inactive!");
        eosio_assert(itr->version == version, "cronjob version updated, old cron schedule stopped!");

        auto num_executions = itr->num_executions + 1;

        call_cron(itr->account, itr->action, version);

        c.modify(itr, ram_payer, [&](auto &i) {
          i.num_executions = num_executions;
          i.updated_at = now();
        });

        call_next(account, itr->interval, itr->version, num_executions|version|current_time());

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

void apply_onerror(uint64_t receiver, const onerror& error ) {
   print("onerror called on ", name{receiver}, "\n");
}

extern "C" {
  [[noreturn]] void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
    croncontract c( receiver );
    c.apply( code, action );
    eosio_exit(0);
  }
}
