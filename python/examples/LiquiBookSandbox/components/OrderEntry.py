import dash
import liquibook
from dash import dcc, Output, Input, State, callback_context, html
import dash_bootstrap_components as dbc
from liquibook_adapter.LiquiBookAdapter import LiquiBookAdapter


class OrderFormComponent(object):

    def __init__(self, app: dash.Dash, liquibook_adapter: LiquiBookAdapter, market_instrument: dict):
        self.liquibook_adapter = liquibook_adapter
        self.app = app
        self.order_details = {}
        self.instrument = market_instrument

        self.register_callbacks()


    def layout(self):
        return html.Div(
            dbc.Card(
            id="order-form-wrapper",
            children=dbc.CardBody([
                dbc.Row([
                    dcc.Store(id="form-ready-store", data=False),
                    dcc.Store(id="selected-price-level", data=False),
                    dbc.Col([
                        dbc.Label("Price"),
                        dbc.Input(type="number", id="price-input", step=100, placeholder="Enter price")
                    ], width=3),
                    dbc.Col([
                        dbc.Label("Quantity"),
                        dbc.Input(type="number", id="quantity-input", step=100, placeholder="Enter quantity", value=500)
                    ], width=3),
                    dbc.Col([
                        dbc.Label("Order Type"),
                        dcc.Dropdown(
                            id="order-type-dropdown",
                            options=[
                                {"label": "Market", "value": "market"},
                                {"label": "Limit", "value": "limit"},
                                {"label": "Stop", "value": "stop"},
                                {"label": "Stop Limit", "value": "stop_limit"},
                            ],
                            style={
                                "color": "black"
                            },
                            value="limit",
                            placeholder="Select order type"
                        )
                    ], width=3),
                    dbc.Col([
                        dbc.Label("Stop Price"),
                        dbc.Input(type="number", id="stop-price-input", step=1, placeholder="Enter stop price",
                                  disabled=True, value=0)
                    ], width=3),
                ], className="mb-3"),
                dbc.Row([
                    dbc.Col([
                        dbc.Label("Condition"),
                        dcc.Dropdown(
                            id="condition-dropdown",
                            options=[
                                {"label": "Day", "value": liquibook.oc_no_conditions},
                                {"label": "Immediate or cancel", "value": liquibook.oc_immediate_or_cancel},
                                {"label": "Fill or Kill", "value": liquibook.oc_fill_or_kill},
                            ],
                            style={
                                "color": "black"
                            },
                            value=liquibook.oc_no_conditions,
                            placeholder="Select condition"
                        )
                    ], width=3),
                    dbc.Col(
                        html.Div([
                            dbc.Label("All or None"),
                            dbc.Checkbox(id="all-or-none-checkbox", value=False)
                        ], className="d-flex align-items-center gap-2 mt-4"),
                        width=3
                    ),
                    dbc.Col([
                        dbc.Button("Buy", id="buy-button", color="success", className="me-2 w-25"),
                        dbc.Button("Sell", id="sell-button", color="danger", className="me-2 w-25"),
                    ], width=6, className="d-flex justify-content-left align-items-center")
                ]),
            ]),
            style={"borderRadius": "10px"},
        ))

    def register_callbacks(self):

        @self.app.callback(
            Output("stop-price-input", "disabled"),
            Input("order-type-dropdown", "value"),
            prevent_initial_call=True
        )
        def toggle_stop_price(order_type):
            if order_type in ["stop", "stop_limit"]:
                return False
            return True

        @self.app.callback(
            Output("liquibook-state-change", "data"),
            Input("buy-button", "n_clicks"),
            Input("sell-button", "n_clicks"),
            State("price-input", "value"),
            State("quantity-input", "value"),
            State("order-type-dropdown", "value"),
            State("stop-price-input", "value"),
            State("condition-dropdown", "value"),
            State("all-or-none-checkbox", "value"),
            prevent_initial_call=True
        )
        def handle_order(buy_clicks, sell_clicks, price, quantity, order_type, stop_price, order_condition,
                 all_or_none):

            triggered = callback_context.triggered_id

            if all_or_none:
                order_condition = order_condition | liquibook.oc_all_or_none

            operation = {
                'operation': 'order',
                'price': price,
                'quantity': quantity,
                'order_type': order_type,
                'stop_price': stop_price or 0,
                'condition': order_condition,
            }

            if triggered == "buy-button":
                operation['is_buy'] = True
            elif triggered == "sell-button":
                operation['is_buy'] = False

            liquibook_state = self.liquibook_adapter.submit_order(operation)

            return liquibook_state

        @self.app.callback(
            Output("price-input", "value"),
            Input("selected-price-level", "data"),
            State("price-input", "value"),
            prevent_initial_call=True
        )
        def price_select(price_depth_level, latest_price):
            if price_depth_level is None:
                if latest_price is not None:
                    return latest_price
                return self.instrument['market_price']
            return price_depth_level