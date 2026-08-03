import liquibook
from liquibook.helpers import pretty_print
from liquibook.helpers import json_serializer
import logging

logger = logging.getLogger("OrderListener")

class OrderListener(liquibook.OrderListener):

    def __init__(self, order_states):
        self.order_states = order_states
        liquibook.OrderListener.__init__(self)

    def on_accept(self, order):
        self.order_states[order.order_id_] = order
        order.accept()
        logger.info(f"Order accepted: {json_serializer.order(order)}")


    def on_reject(self, order):
        self.order_states[order.order_id_] = order
        logger.info(f"Order Rejected: {json_serializer.order(order)}")

    def on_cancel(self, order):
        order.cancel()
        self.order_states[order.order_id_] = order
        logger.info(f"Order Rejected: {json_serializer.order(order)}")

    def on_replace(self, order, size_delta, new_price):
        order.replace(size_delta, new_price)
        self.order_states[order.order_id_] = order
        logger.info(f"Order Rejected: {json_serializer.order(order)}")

    def on_fill(self, passive_order, aggressive_order, fill_qty, fill_cost):
        passive_order.fill(fill_qty, fill_cost, 0)
        aggressive_order.fill(fill_qty, fill_cost, 0)

        self.order_states[passive_order.order_id_] = passive_order
        self.order_states[aggressive_order.order_id_] = aggressive_order

        logger.info(f"Order Fill Passive: {json_serializer.order(passive_order)}")
        logger.info(f"Order Fill Aggressive: {json_serializer.order(aggressive_order)}")
