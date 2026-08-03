import liquibook
import logging
from liquibook.helpers import json_serializer

logger = logging.getLogger("DepthListener")

class DepthListener(liquibook.DepthListener):

    def __init__(self):
        liquibook.DepthListener.__init__(self)
        self.price_depth_cache = {}

    def on_depth_change(self, book, depth):

        logger.info(f"Depth change: {book.symbol()} - {json_serializer.depth(depth)}")

        price_levels_out = []

        for level in range(0, liquibook.DEPTH):
            price_level = {}

            bid = depth.get_bid_levels()[level]
            ask = depth.get_ask_levels()[level]
            price_level['bid_price'] = bid.price()
            price_level['ask_price'] = ask.price()
            price_level['bid_size'] = bid.aggregate_qty()
            price_level['ask_size'] = ask.aggregate_qty()

            price_levels_out.append(price_level)

        self.price_depth_cache = price_levels_out

    def get_price_depth(self):
        return self.price_depth_cache