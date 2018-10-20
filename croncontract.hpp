/**
 *  @file bankofmemory.hpp
 */
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/multi_index.hpp>

#define EOS_SYMBOL S(4, EOS)

using namespace eosio;

namespace cron
{
static const account_name code_account = N(croncontract);
static const account_name ram_payer = N(croncontract);

// @abi table cronjob i64
struct cronjob
{
  account_name account;
  std::string action;

  uint64_t interval;        // interval in seconds
  uint64_t version;         // current schedule version
  uint64_t is_active;       // if this cronjob is still in effect
  uint64_t num_executions;
  uint64_t created_at;      // unix time, in seconds
  uint64_t updated_at;      // unix time, in seconds

  account_name primary_key() const { return account; }

  EOSLIB_SERIALIZE(cronjob, (account)(action)(interval)(version)(is_active)(num_executions)(created_at)(updated_at));
};

typedef multi_index<N(cronjob), cronjob> cronjob_table;

}// namespace cron
