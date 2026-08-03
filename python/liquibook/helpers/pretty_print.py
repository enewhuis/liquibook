# Copyright (c) Mike Kipnis
import liquibook

order_header_meta_data = \
    {'order_id_': {'col': 'ID', 'format': '<'},
     'is_buy': {'col': 'IsBuy', 'format': '>'},
     'state': {'col': 'State', 'format': '>'},
     'price': {'col': 'Price', 'format': '>'},
     'stop_price': {'col': 'StopPrice', 'format': '>'},
     'order_qty': {'col': 'OrderQty', 'format': '>'},
     'open_qty': {'col': 'OpenQty', 'format': '>'},
     'filled_qty': {'col': 'FilledQty', 'format': '>'},
     'conditions': {'col': 'Condition', 'format': '>'},
     'all_or_none': {'col': 'AllOrNone', 'format': '>'},
     'immediate_or_cancel': {'col': 'ImmediateOrCancel', 'format': '>'}}

header_format = ''
col_columns = []
for (key, value) in order_header_meta_data.items():
    header_format += ('{:' + value['format'] + str(len(value['col'])) + '}')
    header_format += ' '
    col_columns.append(value['col'])

order_header = header_format.format(*col_columns)
order_header_separator = '-'*(len(order_header)-1)

depth_header_format = '{bid[price]:>7} - {ask[price]:<7}   {bid[size]:>7} x {ask[size]:<7}'
depth_header = depth_header_format.\
                     format(bid={'price':'Bid','size':'B-Size'}, ask={'price':'Ask','size':'A-Size'})
depth_header_separator = '-'*(len(depth_header)-1)


def depth_level(liquibook_depth_level):

    tuple_out = {}

    if liquibook_depth_level.price() != 0:
        tuple_out['price'] = '{:^7}'.format(liquibook_depth_level.price())
        tuple_out['size'] = '{:^7}'.format(liquibook_depth_level.aggregate_qty())
    else:
        tuple_out['price'] = '{:7}'.format('')
        tuple_out['size'] = '{:7}'.format('')

    return tuple_out


def depth(liquibook_depth):

    top_level_out = ''

    for level in range(0,liquibook.DEPTH):
        bid = liquibook_depth.get_bid_levels()[level]
        ask = liquibook_depth.get_ask_levels()[level]

        bid_price_size_tuple = depth_level(bid)
        ask_price_size_tuple = depth_level(ask)

        top_level_out += depth_header_format. \
                format(bid=bid_price_size_tuple, ask=ask_price_size_tuple) + '\n'

    return top_level_out


def order(order):

    order_data = {
            'order_id_':order.order_id_,
            'is_buy': order.is_buy(),
            'state': order.state(),
            'price': order.price(),
            'stop_price':  order.stop_price(),
            'order_qty': order.order_qty(),
            'open_qty': order.open_qty(),
            'filled_qty': order.filled_qty(),
            'conditions': order.conditions(),
            'all_or_none':  order.all_or_none(),
            'immediate_or_cancel': order.immediate_or_cancel()
    }

    order_values = []
    for (key, value) in order_header_meta_data.items():
        order_values.append(str(order_data[key]))

    return header_format.format(*order_values)
