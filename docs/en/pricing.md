# Pricing and Billing

The local engine is free and complete. Cloud quota and template settlement are incremental
services. Stripe-compatible sandbox billing is implemented but `PHYSFX_BILLING_ENABLED=0` by
default; formal prices and payment availability depend on the operating entity and approval.

Orders, subscriptions, ledger entries, webhook idempotency and manual refund reversals are stored
for reconciliation. When billing is disabled, product behavior is identical to the free edition.
