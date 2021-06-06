/**
 * @brief C-based Helloworld BPF program
 */
#include <solana_sdk.h>
#include "/home/boris/.local/share/solana/install/releases/1.6.10/solana-release/bin/sdk/bpf/c/inc/solana_sdk.h"

//#define DEBUG_INSTRUCTION_DATA

/// Assign account to a program
///
/// # Account references
///   0. [WRITE, SIGNER] Assigned account public key
/// # Payload
///   0. this cmd - uint32_t
///   1. owner - uint64_t    // Owner program account
const static uint32_t SI_ASSIGN = 1;

/// Transfer lamports
///
/// # Account references
///   0. [WRITE, SIGNER] Funding account
///   1. [WRITE] Recipient account
/// # Payload
///   0. this cmd - uint32_t
///   1. lamports - uint64_t
const static uint32_t SI_TRANSFER = 2;

/// Allocate space in a (possibly new) account without funding
///
/// # Account references
///   0. [WRITE, SIGNER] New account
/// # Payload
///   0. this cmd - uint32_t
///   1. space - uint64_t
const static uint32_t SI_ALLOCATE = 8;


/// Initializes a new account to hold tokens.  If this account is associated
/// with the native mint then the token balance of the initialized account
/// will be equal to the amount of SOL in the account. If this account is
/// associated with another mint, that mint must be initialized before this
/// command can succeed.
///
/// The `InitializeAccount` instruction requires no signers and MUST be
/// included within the same Transaction as the system program's
/// `CreateAccount` instruction that creates the account being initialized.
/// Otherwise another party can acquire ownership of the uninitialized
/// account.
///
/// Accounts expected by this instruction:
///
///   0. `[writable]`  The account to initialize.
///   1. `[]` The mint this account will be associated with.
///   2. `[]` The new account's owner/multisignature.
///   3. `[]` Rent sysvar
const static uint8_t TI_INITIALIZE_ACCOUNT = 1;

/// Transfers tokens from one account to another either directly or via a
/// delegate.  If this account is associated with the native mint then equal
/// amounts of SOL and Tokens will be transferred to the destination
/// account.
///
/// Accounts expected by this instruction:
///
///   0. `[writable]` The source account.
///   1. `[writable]` The destination account.
///   2. `[signer]` The source account's owner/delegate.
const static uint8_t TI_TRANSFER = 3;

/// Approves a delegate.  A delegate is given the authority over tokens on
/// behalf of the source account's owner.
///
/// Accounts expected by this instruction:
///
///   * Single owner
///   0. `[writable]` The source account.
///   1. `[]` The delegate.
///   2. `[signer]` The source account owner.
///
///   Approve {
///       `The amount of tokens the delegate is approved for.
///       amount: u64,
///   }
const static uint8_t TI_APPROVE = 4;

typedef enum
{
    /// Account is not yet initialized
    Uninitialized,
    /// Account is initialized; the account owner and/or delegate may perform permitted operations
    /// on this account
    Initialized,
    /// Account has been frozen by the mint freeze authority. Neither the account owner nor
    /// the delegate are able to perform operations on this account.
    Frozen,
} SplAccountState;

typedef struct {
    SolPubkey *mint; // The mint associated with this account
    SolPubkey *owner; // The owner of this account.
    uint64_t   amount;// The amount of tokens this account holds.
    uint32_t   delegate_is_set;
    SolPubkey *delegate; // If `delegate` is `Some` then `delegated_amount` represents
                         // the amount authorized by the delegate
    SplAccountState *state; // The account's state
    uint32_t   native_is_set;
    uint64_t   is_native;  // If is_some, this is a native token, and the value logs the rent-exempt reserve. An Account
                           // is required to be rent-exempt, so the value is used by the Processor to ensure that wrapped
                           // SOL accounts do not drop below this threshold.
    uint64_t   delegated_amount; // The amount delegated
    uint32_t   close_authority_is_set;
    SolPubkey *close_authority; // Optional authority to close the account.
} SplAccount;

const static size_t SPL_TOKEN_ACCOUNT_DATA_LEN = 165;


static bool spl_deserialize(const uint8_t *data, SplAccount *account)
{
    if (NULL == data || NULL == account)
    {
        return false;
    }
    account->mint = (SolPubkey *) data;
    data += SIZE_PUBKEY;

    account->owner = (SolPubkey *) data;
    data += SIZE_PUBKEY;

    account->amount = *(uint64_t *) data;
    data += sizeof (uint64_t);

    account->delegate_is_set = *(uint32_t *) data;
    data += sizeof (uint32_t);

    account->delegate = (SolPubkey *) NULL;
    if (account->delegate_is_set)
    {
        account->delegate = (SolPubkey *) data;
    }
    data += SIZE_PUBKEY;

    account->state = (SplAccountState *) data;
    data += sizeof (uint8_t);

    account->native_is_set  = *(uint32_t *) data;
    data += sizeof (uint32_t);

    account->is_native = 0;
    if (account->native_is_set)
    {
        account->is_native = *(uint64_t *) data;
    }
    data += sizeof (uint64_t);

    account->delegated_amount = *(uint64_t *) data;
    data += sizeof (uint64_t);

    account->close_authority_is_set = *(uint32_t *) data;
    data += sizeof (uint32_t);

    account->close_authority = (SolPubkey *) NULL;
    if (account->close_authority_is_set)
    {
        account->close_authority = (SolPubkey *) data;
    }
    data += SIZE_PUBKEY;

    return true;
}

static void spl_log_account(const SplAccount *account)
{
    sol_log("SPL account info:");
    sol_log("The mint associated with this account");
    sol_log_pubkey(account->mint);
    sol_log("The owner of this account");
    sol_log_pubkey(account->owner);
    sol_log("The amount of tokens this account holds");
    sol_log_64(0,0,0,0, account->amount);
    if (account->delegate_is_set)
    {
        sol_log("The delegate of this account");
        sol_log_pubkey(account->delegate);
    }
    else sol_log("The delegate of this account is not set");
    sol_log("The account's state");
    sol_log_64(0,0,0,0, *account->state);
    if (account->native_is_set)
    {
        sol_log("Native token");
        sol_log_64(0,0,0,0, account->is_native);
    }
    else sol_log("Native token is not set");
    sol_log("The amount delegated");
    sol_log_64(0,0,0,0, account->delegated_amount);
    if (account->close_authority_is_set)
    {
        sol_log("Optional authority to close the account");
        sol_log_pubkey(account->close_authority);
    }
    else sol_log("Optional authority to close the account not set");
}

typedef enum
{
    UI_CreatePool,   // 0
    UI_EmptyPool,    // 1
    UI_RemovePool,   // 2
    UI_AddUser,      // 3
    UI_RemoveUser,   // 4
    UI_SetScore,     // 5
    UI_CleanStorage  // 6
} UpalaInstruction;

static uint64_t transfer_to_ata(SolAccountInfo *payer,
                                SolAccountInfo *ata,
                                SolAccountInfo *system_program);

static uint64_t allocate_space_for_ata(      SolAccountInfo *ata,
                                       const SolSignerSeed  *ata_signer_seeds,
                                             uint64_t        ata_signer_seeds_len,
                                             SolAccountInfo *system_program,
                                             uint64_t        allocated_space);

static uint64_t assign_ata(      SolAccountInfo *ata,
                           const SolSignerSeed  *ata_signer_seeds,
                                 uint64_t        ata_signer_seeds_len,
                                 SolAccountInfo *system_program,
                                 SolAccountInfo *spl_token);

static uint64_t initialize_ata(      SolAccountInfo *ata,
                                     SolAccountInfo *minter,
                               const SolAccountInfo *owner,
                                     SolAccountInfo *sysvar_rent,
                                     SolAccountInfo *spl_token);

static uint64_t create_pool_id(SolAccountInfo *payer,
                               SolAccountInfo *ata,
                               SolAccountInfo *minter,
                               SolAccountInfo *owner,
                               SolAccountInfo *system_program,
                               SolAccountInfo *sysvar_rent,
                               SolAccountInfo *spl_token,
                         const SolSignerSeed  *ata_signer_seeds,
                               uint64_t        ata_signer_seeds_len);

typedef struct
{
    SolPubkey            key;
    const SolSignerSeed *seed;
    uint64_t             seed_len;
    uint8_t              bump_seed;
} SolInnerAccount;

typedef struct
{
    UpalaInstruction  *instriction;
    uint64_t           data_len;
    uint8_t           *data;
} UpalaInstractionData;

typedef struct
{
    SolPubkey  key;
    uint64_t   score;
} UpalaAccount;

typedef struct
{
    SolPubkey     key;
    SolPubkey     manager;
    uint8_t       accounts_count;
    UpalaAccount  accounts[10];
} UpalaGroup;

static bool upala_deserialize(const uint8_t *data, UpalaInstractionData *ui_data)
{
    if (NULL == data || NULL == ui_data)
    {
        return false;
    }

    ui_data->instriction = (UpalaInstruction*) data;
    data += sizeof (uint8_t);

    ui_data->data_len = *(uint64_t*) data;
    data += sizeof (uint64_t);

    if (*ui_data->instriction == UI_CreatePool)
    {
        // NOP
    }
    else if (*ui_data->instriction == UI_EmptyPool)
    {
        // NOP
    }


    return true;
}


static void upala_log_group(const uint8_t *_data, const SolPubkey *gid)
{
    uint8_t *d = (uint8_t *)sol_calloc(MAX_PERMITTED_DATA_INCREASE, sizeof (uint8_t));
    sol_memcpy(d, _data, MAX_PERMITTED_DATA_INCREASE);

    const uint8_t groups_count = *(uint8_t*)d;
    d += sizeof (uint8_t);

    sol_log("Number of groups: ->");
    sol_log_64(0,0,0,0, groups_count);

    bool finded_ug = false;
    UpalaGroup *ug = NULL;
    for (size_t i = 0; i < groups_count; i++)
    {
        ug = (UpalaGroup *) d;
        if (SolPubkey_same(&ug->key, gid))
        {
            finded_ug = true;
            break;
        }
        d += sizeof (UpalaGroup);
    }

    if (finded_ug)
    {
        sol_log("Group id: ->");
        sol_log_pubkey(&ug->key);
        sol_log("Group manager id: ->");
        sol_log_pubkey(&ug->manager);
        sol_log("Num of accounts: ->");
        sol_log_64(0,0,0,0, ug->accounts_count);
        // -----------------------------------

        sol_log("#Users");
        for (size_t j = 0; j < ug->accounts_count; j++)
        {
            UpalaAccount uas = ug->accounts[j];
            sol_log("User id: ->");
            sol_log_pubkey(&uas.key);
            sol_log("The score of user: ->");
            sol_log_64(0,0,0,0, uas.score);
        }
    }
}

uint64_t processing(SolParameters *params)
{
    if (params->ka_num < 7)
    {
        sol_log("Greeted account not included in the instruction");
        return ERROR_NOT_ENOUGH_ACCOUNT_KEYS;
    }

    // Get accounts
    SolAccountInfo *manager_account             = &params->ka[0];
    SolAccountInfo *pool_at_account             = &params->ka[1];
    SolAccountInfo *pools_manager_account       = &params->ka[2];

    SolAccountInfo *minter_account              = &params->ka[3];
    SolAccountInfo *upala_account               = &params->ka[4];
    SolAccountInfo *system_program_account      = &params->ka[5];
    SolAccountInfo *sysvar_rent_account         = &params->ka[6];
    SolAccountInfo *spl_token_account           = &params->ka[7];

    SolAccountInfo *user_account                = &params->ka[8];
    SolAccountInfo *user_at_account             = &params->ka[9];

    // The account must be owned by the program in order to modify its data
    if (!SolPubkey_same(manager_account->owner, system_program_account->key))
    {
        return ERROR_INCORRECT_PROGRAM_ID;
    }

    if (params->data_len == 0)
    {
        return ERROR_INVALID_INSTRUCTION_DATA;
    }

    SolInnerAccount *ata = (SolInnerAccount *) sol_calloc(1, sizeof (SolInnerAccount));
    {
        const uint64_t ata_seeds_count  = 4;
        SolSignerSeed* ata_seeds        = (SolSignerSeed*)sol_calloc(ata_seeds_count, sizeof (SolSignerSeed));

        ata_seeds[0] = (SolSignerSeed){manager_account->key->x, SIZE_PUBKEY};
        ata_seeds[1] = (SolSignerSeed){minter_account->key->x, SIZE_PUBKEY};
        ata_seeds[2] = (SolSignerSeed){upala_account->key->x, SIZE_PUBKEY};
        ata_seeds[3] = (SolSignerSeed){0, 0};

        sol_try_find_program_address(ata_seeds, (ata_seeds_count - 1),
                                     upala_account->key,
                                     &ata->key, &ata->bump_seed);
        sol_log("Finded associated token account id:");
        sol_log_pubkey(&ata->key);

        if (!SolPubkey_same(pool_at_account->key, &ata->key))
        {
            sol_log("Error: Associated address does not match seed derivation");
            return INVALID_SEEDS;
        }

        ata_seeds[ata_seeds_count - 1] = (SolSignerSeed){&ata->bump_seed, 1};
        ata->seed     = (const SolSignerSeed*)ata_seeds;
        ata->seed_len = ata_seeds_count;
    }

    SolInnerAccount *pta = (SolInnerAccount *) sol_calloc(1, sizeof (SolInnerAccount));
    {
        const uint64_t pta_seeds_count  = 3;
        SolSignerSeed* pta_seeds        = (SolSignerSeed*)sol_calloc(pta_seeds_count, sizeof (SolSignerSeed));

        pta_seeds[0] = (SolSignerSeed){minter_account->key->x, SIZE_PUBKEY};
        pta_seeds[1] = (SolSignerSeed){upala_account->key->x, SIZE_PUBKEY};
        pta_seeds[2] = (SolSignerSeed){0, 0};

        sol_try_find_program_address(pta_seeds, (pta_seeds_count - 1),
                                     upala_account->key,
                                     &pta->key, &pta->bump_seed);
        sol_log("Finded associated token account id:");
        sol_log_pubkey(&pta->key);

        if (!SolPubkey_same(pools_manager_account->key, &pta->key))
        {
            sol_log("Error: Associated address does not match seed derivation");
            return INVALID_SEEDS;
        }

        pta_seeds[pta_seeds_count - 1] = (SolSignerSeed){&pta->bump_seed, 1};
        pta->seed     = (const SolSignerSeed*)pta_seeds;
        pta->seed_len = pta_seeds_count;

        //init pools_manager_account
        if (!SolPubkey_same(pools_manager_account->owner, params->program_id))
        {
            transfer_to_ata(manager_account, pools_manager_account, system_program_account);
            allocate_space_for_ata(pools_manager_account, pta->seed, pta->seed_len, system_program_account, MAX_PERMITTED_DATA_INCREASE);
            assign_ata(pools_manager_account, pta->seed, pta->seed_len, system_program_account, upala_account);

//            sol_memset(storage, 0, MAX_PERMITTED_DATA_INCREASE);
        }
    }

    uint8_t * storage = pools_manager_account->data;

    const uint8_t upala_instriction_ptr = *(uint8_t *)params->data; params->data += sizeof (uint8_t);
    UpalaInstruction upala_instriction = (UpalaInstruction)upala_instriction_ptr;

    sol_log_64(0,0,0,upala_instriction_ptr,upala_instriction);
    if (upala_instriction == UI_CreatePool)
    {
        sol_log("Called the instruction UI_CreatePool");
        uint64_t return_value = SUCCESS;
        if (!SolPubkey_same(pool_at_account->owner, spl_token_account->key))
        {
            //init associated_token_account
            return_value = transfer_to_ata(manager_account, pool_at_account, system_program_account);
            return_value = allocate_space_for_ata(pool_at_account, ata->seed, ata->seed_len, system_program_account, SPL_TOKEN_ACCOUNT_DATA_LEN);
            return_value = assign_ata(pool_at_account, ata->seed, ata->seed_len, system_program_account, spl_token_account);
            return_value = initialize_ata(pool_at_account, minter_account, pools_manager_account, sysvar_rent_account, spl_token_account);

            uint8_t groups_count = *(uint8_t*)storage;
            groups_count += 1;
            sol_memcpy(storage, &groups_count, sizeof(uint8_t));

            UpalaGroup ug;
            ug.key = *pool_at_account->key;
            ug.manager = *manager_account->key;
            ug.accounts_count = 0;
            sol_memcpy(storage + sizeof(uint8_t) + sizeof(SolPubkey) * (groups_count-1),
                       &ug, sizeof(UpalaGroup));
            sol_log("Group created");
        }
        else sol_log("The group exists");

        upala_log_group(storage, pool_at_account->key);

        return return_value;
    }
    else if (upala_instriction == UI_EmptyPool)
    {   sol_log("Called the instruction UI_EmptyPool");

        /*SolInnerAccount *dta = (SolInnerAccount *) sol_calloc(1, sizeof (SolInnerAccount));
        {
            const uint64_t dta_seeds_count  = 4;
            SolSignerSeed* dta_seeds        = (SolSignerSeed*)sol_calloc(dta_seeds_count, sizeof (SolSignerSeed));

            dta_seeds[0] = (SolSignerSeed){u_account->key->x, SIZE_PUBKEY};
            dta_seeds[1] = (SolSignerSeed){minter_account->key->x, SIZE_PUBKEY};
            dta_seeds[2] = (SolSignerSeed){upala_account->key->x, SIZE_PUBKEY};
            dta_seeds[3] = (SolSignerSeed){0, 0};

            sol_try_find_program_address(dta_seeds, (dta_seeds_count - 1),
                                         upala_account->key,
                                         &dta->key, &dta->bump_seed);
            sol_log("Finded associated token account id:");
            sol_log_pubkey(&dta->key);

            if (!SolPubkey_same(user_account->key, &dta->key))
            {
                sol_log("Error: Associated address does not match seed derivation");
                return INVALID_SEEDS;
            }

            dta_seeds[dta_seeds_count - 1] = (SolSignerSeed){&dta->bump_seed, 1};
            dta->seed     = (const SolSignerSeed*)dta_seeds;
            dta->seed_len = dta_seeds_count;
        }*/

        sol_log("User SPL account");
        SplAccount user_spl_info;
        spl_deserialize(user_at_account->data, &user_spl_info);
        spl_log_account(&user_spl_info);

        sol_log("Pool SPL account");
        SplAccount pool_spl_info;
        spl_deserialize(pool_at_account->data, &pool_spl_info);
        spl_log_account(&pool_spl_info);

        /// 0. `[writable]` The source account.
        /// 1. `[writable]` The destination account.
        /// 2. `[signer]` The source account's owner/delegate.
        SolAccountMeta arguments[] = {
            {.pubkey = pool_at_account->key,          .is_writable = true,  .is_signer = false},
            {.pubkey = user_at_account->key,          .is_writable = true,  .is_signer = false},
            {.pubkey = pools_manager_account->key,    .is_writable = false, .is_signer = true},
        };

        uint8_t cmd = TI_TRANSFER;
        uint64_t amount = (uint64_t)(pool_spl_info.amount);

        uint8_t data[sizeof (cmd) + sizeof(amount)];
        sol_memcpy(data, &cmd, sizeof(cmd));
        sol_memcpy(data + sizeof(cmd), &amount, sizeof(amount));
#ifdef DEBUG_INSTRUCTION_DATA
        sol_log_array(data, SOL_ARRAY_SIZE(data));
#endif

        const SolInstruction instruction = {
            spl_token_account->key,
            arguments, SOL_ARRAY_SIZE(arguments),
            data, SOL_ARRAY_SIZE(data)
        };

        const SolAccountInfo account_infos[] = {
            *pool_at_account,
            *user_at_account,
            *pools_manager_account,
            *spl_token_account,
        };

        const SolSignerSeeds signers_seeds[] = { {pta->seed, pta->seed_len} };

        return sol_invoke_signed(&instruction,
                                 account_infos, SOL_ARRAY_SIZE(account_infos),
                                 signers_seeds, SOL_ARRAY_SIZE(signers_seeds));
    }
    else if (upala_instriction == UI_AddUser)
    {
        /*{
            uint8_t *data_ptr = (uint8_t *)sol_calloc(MAX_PERMITTED_DATA_INCREASE, sizeof (uint8_t));
            sol_memcpy(data_ptr, storage, MAX_PERMITTED_DATA_INCREASE);

            uint8_t groups_count = *(uint8_t*)data_ptr;
            data_ptr += sizeof (uint8_t);
            sol_log("=== Groups in data ===");
            sol_log_64(0,0,0,0,groups_count);

            for (size_t i = 0; i < groups_count; i++)
            {
                UpalaGroup *ug = (UpalaGroup *) data_ptr;
                sol_log("Group id: ->");
                sol_log_pubkey(&ug->key);
                sol_log("Group manager id: ->");
                sol_log_pubkey(&ug->manager);
                sol_log("Num of accounts: ->");
                sol_log_64(0,0,0,0, ug->accounts_count);
                data_ptr += sizeof (UpalaGroup);
                sol_log("*-*-*-*-*-*");
            }
        }*/

        if (!SolPubkey_same(user_at_account->owner, spl_token_account->key))
        {
            SolInnerAccount *user_ata = (SolInnerAccount *) sol_calloc(1, sizeof (SolInnerAccount));
            {
                const uint64_t uset_ata_seeds_count  = 4;
                SolSignerSeed* uset_ata_seeds        = (SolSignerSeed*)sol_calloc(uset_ata_seeds_count, sizeof (SolSignerSeed));

                uset_ata_seeds[0] = (SolSignerSeed){user_account->key->x, SIZE_PUBKEY};
                uset_ata_seeds[1] = (SolSignerSeed){minter_account->key->x, SIZE_PUBKEY};
                uset_ata_seeds[2] = (SolSignerSeed){upala_account->key->x, SIZE_PUBKEY};
                uset_ata_seeds[3] = (SolSignerSeed){0, 0};

                sol_try_find_program_address(uset_ata_seeds, (uset_ata_seeds_count - 1),
                                             upala_account->key,
                                             &user_ata->key, &user_ata->bump_seed);
                sol_log("Finded associated token account id:");
                sol_log_pubkey(&user_ata->key);

                if (!SolPubkey_same(user_at_account->key, &user_ata->key))
                {
                    sol_log("Error: Associated address does not match seed derivation");
                    return INVALID_SEEDS;
                }

                uset_ata_seeds[uset_ata_seeds_count - 1] = (SolSignerSeed){&user_ata->bump_seed, 1};
                user_ata->seed     = (const SolSignerSeed*)uset_ata_seeds;
                user_ata->seed_len = uset_ata_seeds_count;
            }

            //init associated_token_account for new user
            transfer_to_ata(manager_account, user_at_account, system_program_account);
            allocate_space_for_ata(user_at_account, user_ata->seed, user_ata->seed_len, system_program_account, SPL_TOKEN_ACCOUNT_DATA_LEN);
            assign_ata(user_at_account, user_ata->seed, user_ata->seed_len, system_program_account, spl_token_account);
            initialize_ata(user_at_account, minter_account, user_account, sysvar_rent_account, spl_token_account);

            sol_log("Create associated_token_account for new user");
        }

//        SplAccount spl_info;
//        spl_deserialize(pool_at_account->data, &spl_info);
//        spl_log_account(&spl_info);

        const SolPubkey *gid = (SolPubkey *)params->data;
        params->data += SIZE_PUBKEY;

        const uint8_t uids_count = *(uint8_t *)params->data;
        params->data += sizeof (uint8_t);

        /*sol_log("=== From client: === ");
        sol_log("....GID:");
        sol_log_pubkey(gid);
        sol_log("....UIDS_COUNT:");
        sol_log_64(0,0,0,0,uids_count);*/

        uint8_t groups_count = *(uint8_t*)storage;
        storage += sizeof (uint8_t);

        bool finded_ug = false;
        UpalaGroup *ug = NULL;
        for (size_t i = 0; i < groups_count; i++)
        {
            ug = (UpalaGroup *) storage;
            if (SolPubkey_same(&ug->key, gid))
            {
                finded_ug = true;
                break;
            }
            storage += sizeof (UpalaGroup);
        }

        if (finded_ug)
        {
            sol_log("Group id: ->");
            sol_log_pubkey(&ug->key);
            sol_log("Group manager id: ->");
            sol_log_pubkey(&ug->manager);
            sol_log("Num of accounts: ->");
            sol_log_64(0,0,0,0, ug->accounts_count);
            // -----------------------------------

            sol_log("Adding account");

            for (size_t i = 0; i < uids_count; i++)
            {
                SolPubkey uid = *(SolPubkey *) params->data;
                params->data += SIZE_PUBKEY;
                sol_log("New user id: ->");
                sol_log_pubkey(&uid);

                uint64_t score = *(uint64_t *) params->data;
                params->data += sizeof (uint64_t);
                sol_log("The score of the new user: ->");
                sol_log_64(0,0,0,0, score);

                UpalaAccount *uas = (UpalaAccount *)ug->accounts;
                uas[ug->accounts_count++] = (UpalaAccount){uid, score};
            }

            // -----------------------------------

            sol_log("=== All users ===");
            for (size_t j = 0; j < ug->accounts_count && j < 10; j++)
            {
                UpalaAccount uas = ug->accounts[j];
                sol_log("...User id: ->");
                sol_log_pubkey(&uas.key);
                sol_log("...The score of user: ->");
                sol_log_64(0,0,0,0, uas.score);
            }
        }
    }
    else if (upala_instriction == UI_CleanStorage)
    {
        sol_memset(storage, 0, MAX_PERMITTED_DATA_INCREASE);
    }

    return SUCCESS;
}

static uint64_t transfer_to_ata(SolAccountInfo *payer,
                                SolAccountInfo *ata,
                                SolAccountInfo *system_program)
{
    sol_log("Transfer 2039280 lamports to the new associated token account:");

    const uint64_t required_lamports = 2039280;

    SolAccountMeta accounts[] = {
        {.pubkey = payer->key,  .is_writable = true, .is_signer = true},
        {.pubkey = ata->key,    .is_writable = true, .is_signer = false}
    };

    uint32_t cmd = SI_TRANSFER; // SystemInstruction::Transfer
    uint8_t data[sizeof (uint32_t) + sizeof(required_lamports)];
    sol_memcpy(data, &cmd, sizeof (uint32_t));
    sol_memcpy(data + sizeof (uint32_t), &required_lamports, sizeof(required_lamports));
#ifdef DEBUG_INSTRUCTION_DATA
    sol_log_array(data, SOL_ARRAY_SIZE(data));
#endif

    const SolInstruction instruction = {
        .program_id     = system_program->key,
        .accounts       = accounts,
        .account_len    = SOL_ARRAY_SIZE(accounts),
        .data           = data,
        .data_len       = SOL_ARRAY_SIZE(data)
    };

    const SolAccountInfo account_infos[] = {
        *payer,
        *ata,
        *system_program
    };

    return sol_invoke(&instruction, account_infos, SOL_ARRAY_SIZE(account_infos));
}

static uint64_t allocate_space_for_ata(      SolAccountInfo *ata,
                                       const SolSignerSeed  *ata_signer_seeds,
                                             uint64_t        ata_signer_seeds_len,
                                             SolAccountInfo *system_program,
                                             uint64_t        allocated_space)
{
    sol_log("Allocate space for the associated token account");

    SolAccountMeta arguments[] = {
        // [WRITE, SIGNER] New account
        {.pubkey = ata->key, .is_writable = true, .is_signer = true}
    };

    uint32_t cmd = SI_ALLOCATE;
    uint64_t required_space = allocated_space;
    uint8_t data[sizeof (cmd) + sizeof(required_space)];
    sol_memcpy(data, &cmd, sizeof (cmd));
    sol_memcpy(data + sizeof (cmd), &required_space, sizeof(required_space));
#ifdef DEBUG_INSTRUCTION_DATA
    sol_log_array(data, SOL_ARRAY_SIZE(data));
#endif

    const SolInstruction instruction = {
        system_program->key,
        arguments, SOL_ARRAY_SIZE(arguments),
        data, SOL_ARRAY_SIZE(data)
    };

    const SolSignerSeeds signers_seeds[] = {
        {ata_signer_seeds, ata_signer_seeds_len}
    };

    const SolAccountInfo account_infos[] = {
        *ata,
        *system_program
    };

    uint64_t err = sol_invoke_signed(&instruction,
                      account_infos, SOL_ARRAY_SIZE(account_infos),
                      signers_seeds, SOL_ARRAY_SIZE(signers_seeds));

    // So that an error does not appear: "failed: non-system instruction changed account size"
    ata->data_len = required_space;
    sol_memset(ata->data, 0, required_space);

    return err;
}

static uint64_t assign_ata(      SolAccountInfo *ata,
                           const SolSignerSeed  *ata_signer_seeds,
                                 uint64_t        ata_signer_seeds_len,
                                 SolAccountInfo *system_program,
                                 SolAccountInfo *spl_token)
{
    sol_log("Assign the associated token account to the SPL Token program");

    SolAccountMeta arguments[] = {
        // [WRITE, SIGNER] Assigned account public key
        {.pubkey = ata->key, .is_writable = true, .is_signer = true}
    };

    uint32_t cmd = SI_ASSIGN;
    uint8_t data[sizeof(cmd) + SIZE_PUBKEY];
    sol_memcpy(data, &cmd, sizeof(cmd));
    sol_memcpy(data + sizeof(cmd), spl_token->key->x, SIZE_PUBKEY);
#ifdef DEBUG_INSTRUCTION_DATA
    sol_log_array(data, SOL_ARRAY_SIZE(data));
#endif

    const SolInstruction instruction = {
        system_program->key,
        arguments, SOL_ARRAY_SIZE(arguments),
        data, SOL_ARRAY_SIZE(data)
    };

    const SolSignerSeeds signers_seeds[] = {
        {ata_signer_seeds, ata_signer_seeds_len}
    };

    const SolAccountInfo account_infos[] = {
        *ata,
    };

    return sol_invoke_signed(&instruction,
                      account_infos, SOL_ARRAY_SIZE(account_infos),
                      signers_seeds, SOL_ARRAY_SIZE(signers_seeds));
}

static uint64_t initialize_ata(      SolAccountInfo *ata,
                                     SolAccountInfo *minter,
                               const SolAccountInfo *owner,
                                     SolAccountInfo *sysvar_rent,
                                     SolAccountInfo *spl_token)
{
    sol_log("Initialize the associated token account");

    SolAccountMeta arguments[] = {
        ///   0. `[writable]`  The account to initialize.
        {.pubkey = ata->key, .is_writable = true, .is_signer = false},
        ///   1. `[]` The mint this account will be associated with.
        {.pubkey = minter->key, .is_writable = false, .is_signer = false},
        ///   2. `[]` The new account's owner/multisignature.
        {.pubkey = (SolPubkey *)owner->key, .is_writable = false, .is_signer = false},
        ///   3. `[]` Rent sysvar
        {.pubkey = sysvar_rent->key, .is_writable = false, .is_signer = false}
    };

    uint32_t cmd = TI_INITIALIZE_ACCOUNT;
    uint8_t data[sizeof (cmd)];
    sol_memcpy(data, &cmd, sizeof (cmd));
#ifdef DEBUG_INSTRUCTION_DATA
    sol_log_array(data, SOL_ARRAY_SIZE(data));
#endif

    const SolInstruction instruction = {
        spl_token->key,
        arguments, SOL_ARRAY_SIZE(arguments),
        data, SOL_ARRAY_SIZE(data)
    };

    const SolAccountInfo account_infos[] = {
        *ata,
        *minter,
        *owner,
        *sysvar_rent,
        *spl_token
    };

    return sol_invoke(&instruction, account_infos, SOL_ARRAY_SIZE(account_infos));
}

extern uint64_t entrypoint(const uint8_t *input)
{
    SolAccountInfo accounts[10];
    SolParameters params = (SolParameters){.ka = accounts};

    if (!sol_deserialize(input, &params, SOL_ARRAY_SIZE(accounts)))
    {
        return ERROR_INVALID_ARGUMENT;
    }

    return processing(&params);
}
