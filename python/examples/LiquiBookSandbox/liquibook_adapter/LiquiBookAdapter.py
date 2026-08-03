import json
import liquibook
from liquibook_adapter import DepthListener
from liquibook_adapter import BBOListener
from liquibook_adapter import OrderListener
from liquibook.helpers import json_serializer
import logging

logger = logging.getLogger("LiquiBookAdapter")

class LiquiBookAdapter:

    def __init__(self, sandbox_instrument):
        self.order_states = {}
        self.depth_listener = DepthListener.DepthListener()
        self.bbo_listener = BBOListener.BBOListener()
        self.order_listener = OrderListener.OrderListener(self.order_states)

        self.price_depth_book = liquibook.DepthOrderBook()
        self.price_depth_book.set_market_price(sandbox_instrument['market_price'])
        self.price_depth_book.set_symbol(sandbox_instrument['symbol'])
        self.price_depth_book.set_depth_listener(self.depth_listener)
        self.price_depth_book.set_bbo_listener(self.bbo_listener)
        self.price_depth_book.set_order_listener(self.order_listener)

    def update_order_book_data(self):
        return {
            'price_depth': self.depth_listener.get_price_depth(),
            'orders': [
                json_serializer.order(order)
                for order_id, order in self.order_states.items()
            ]}


    def submit_order(self, order_props: dict):
        order = liquibook.SimpleOrder(order_props['is_buy'], order_props['price'], order_props['quantity'], order_props['stop_price'],
                                      order_props['condition'])

        logger.info(f"Order submitted: {json_serializer.order(order)}")

        self.order_states[order.order_id_] = order
        self.price_depth_book.add(order, order_props['condition'])

        return self.update_order_book_data()

    def cancel_order(self, cancel_order):

        order_to_cancel = self.order_states[cancel_order['order_id_']]
        self.price_depth_book.cancel(order_to_cancel)

        logger.info(f"Order cancelled: {json_serializer.order(order_to_cancel)}")

        return self.update_order_book_data()

    def modify_order(self, order_to_modify, size_delta, new_price):
        order_to_modify = self.order_states[order_to_modify['order_id_']]
        self.price_depth_book.replace(order_to_modify, size_delta, new_price)

        logger.info(f"Order modified: {json_serializer.order(order_to_modify)}")

        return self.update_order_book_data()

