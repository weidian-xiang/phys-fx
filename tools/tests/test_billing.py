# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

import hashlib
import hmac
import json

import pytest

from cloud.server.app import Store


def test_billing_is_closed_by_default(tmp_path) -> None:
    store = Store(tmp_path)
    key = store.register("billing@example.com")
    assert store.billing_plans()["enabled"] is False
    with pytest.raises(RuntimeError):
        store.create_order(key, "pro", "one")


def test_sandbox_billing_idempotency_reconciliation_and_refund(tmp_path) -> None:
    store = Store(tmp_path, billing_enabled=True, webhook_secret="secret", admin_key="admin")
    key = store.register("billing@example.com")
    first = store.create_order(key, "pro", "same-order")
    second = store.create_order(key, "pro", "same-order")
    assert first["id"] == second["id"]
    payload = {"id": "evt-1", "type": "payment_succeeded", "order_id": first["id"]}
    body = json.dumps(payload, ensure_ascii=False, separators=(",", ":"), sort_keys=True).encode()
    signature = hmac.new(b"secret", body, hashlib.sha256).hexdigest()
    assert store.apply_webhook(payload, signature)["duplicate"] is False
    assert store.apply_webhook(payload, signature)["duplicate"] is True
    paid = store.db.execute("SELECT state FROM orders WHERE id=?", (first["id"],)).fetchone()
    assert paid["state"] == "paid"
    refund = store.refund("admin", first["id"], "sandbox reconciliation")
    assert refund["amount_cents"] == 1999
    assert store.reconciliation()


def test_queue_reservation_and_cancel_refund(tmp_path) -> None:
    store = Store(tmp_path, quota_seconds=100)
    key = store.register("queue@example.com")
    submitted = store.submit(key, {"input": "clip.mp4", "estimated_seconds": 60})
    assert submitted["queue_position"] == 1
    with pytest.raises(RuntimeError):
        store.submit(key, {"input": "clip-2.mp4", "estimated_seconds": 50})
    store.cancel(key, submitted["job_id"])
    assert store.submit(key, {"input": "clip-3.mp4", "estimated_seconds": 60})["queue_position"] == 1
