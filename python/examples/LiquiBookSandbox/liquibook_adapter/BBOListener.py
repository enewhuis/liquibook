import liquibook
from liquibook.helpers import json_serializer
import logging

logger = logging.getLogger("BBOListener")

class BBOListener(liquibook.DepthOrderBookBboListener):

    def __init__(self):
        liquibook.DepthOrderBookBboListener.__init__(self)

    def on_bbo_change(self, book, depth):
        logger.info(f"BBO change: {book.symbol()} - {json_serializer.bbo(depth)}")