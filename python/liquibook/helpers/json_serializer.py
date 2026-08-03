# Copyright (c) Mike Kipnis
import liquibook

def depth(liquibook_depth):
    price_levels = []

    for level in range(0, liquibook.DEPTH):
        price_level = {}

        bid = liquibook_depth.get_bid_levels()[level]
        ask = liquibook_depth.get_ask_levels()[level]

        price_level['bid_price'] = bid.price()
        price_level['ask_price'] = ask.price()
        price_level['bid_size'] = bid.aggregate_qty()
        price_level['ask_size'] = ask.aggregate_qty()

        price_levels.append(price_level)

    return price_levels

def bbo(top_level):

    top_level_out = {'bid_price': top_level.bids().price(), 'bid_size': top_level.bids().aggregate_qty(),
                     'ask_price': top_level.asks().price(), 'ask_size': top_level.asks().aggregate_qty()}

    return top_level_out

order_state_map = {
        liquibook.os_new:'New',
        liquibook.os_accepted:'Accepted',
        liquibook.os_complete:'Filled',
        liquibook.os_cancelled:'Cancelled',
        liquibook.os_rejected:'Rejected'
}

order_condition_map = {
    liquibook.oc_no_conditions:'No condition',
    liquibook.oc_all_or_none:'All or None',
    liquibook.oc_immediate_or_cancel:'Immediate or Cancel',
    liquibook.oc_fill_or_kill:'Fill or Kill',
    liquibook.oc_stop:'Stop'
}

def order(order_in):

    order_json = {
            'order_id_':order_in.order_id_,
            'is_buy': order_in.is_buy(),
            'state': order_in.state(),
            'state_str': order_state_map[order_in.state()],
            'price': order_in.price(),
            'stop_price':  order_in.stop_price(),
            'order_qty': order_in.order_qty(),
            'open_qty': order_in.open_qty(),
            'filled_qty': order_in.filled_qty(),
            'conditions': order_in.conditions(),
            'conditions_str': order_condition_map[order_in.conditions()],
            'all_or_none':  order_in.all_or_none(),
            'immediate_or_cancel': order_in.immediate_or_cancel()
    }

    return order_json
