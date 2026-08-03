import dash
from dash import dcc, Output, Input, State,  callback_context, html
import dash_bootstrap_components as dbc
from liquibook_adapter.LiquiBookAdapter import LiquiBookAdapter


class OrderCencelModifyComponent(object):

    def __init__(self, app: dash.Dash, liquibook_adapter: LiquiBookAdapter):
        self.liquibook_adapter = liquibook_adapter
        self.app = app
        self.register_callbacks()

    def layout(self):
        return html.Div([
            html.Hr(
                style={
                    "borderTop": "2px solid #6c757d",
                    "margin": "10px 0 10px"
                }
            ),
            dbc.Card(
                dbc.CardBody(
                    dbc.Row([
                        dcc.Store(id="selected-order"),

                        # Order Id
                        dbc.Col(dbc.Label("Order Id:", className="text-end w-100"), width="auto",
                                className="d-flex align-items-center"),
                        dbc.Col(dbc.Label(id="order_id_", className="text-end w-100"), width="auto",
                                className="d-flex align-items-center"),

                        dbc.Col(width=True),  # spacer

                        # Price
                        dbc.Col(dbc.Label("Price", className="text-end w-100"), width="auto",
                                className="d-flex align-items-center"),
                        dbc.Col(dbc.Input(type="number", id="new-price-input", step=100, placeholder="Enter new price"),
                                width=2),

                        # Delta Qty
                        dbc.Col(dbc.Label("Delta Qty", className="text-end w-100"), width="auto",
                                className="d-flex align-items-center"),
                        dbc.Col(
                            dbc.Input(type="number", id="delta-quantity-input", step=100, placeholder="Enter quantity"),
                            width=2),

                        # Buttons
                        dbc.Col([
                            dbc.Button("Modify", id="modify-order-button", color="success", className="me-2"),
                            dbc.Button("Cancel", id="cancel-order-button", color="danger", className="me-2"),
                        ], width="auto", className="d-flex align-items-center"),
                    ], className="g-2 flex-nowrap"),  # prevents wrapping
                ),
                style={"borderRadius": "10px", "margin": "10px 0"},
            ),
        ], id="modify-cancel-panel")

    def register_callbacks(self):

        @self.app.callback(
            Output("order_id_", "children"),
            Output("new-price-input", "value"),
            Output("delta-quantity-input", "value"),
            Output("modify-cancel-panel", "style"),
            Input("selected-order", "data"),
            prevent_initial_call=True
        )
        def handle_selected(selected_order):
            if selected_order is not None and selected_order['state_str'] == 'Accepted':
                return selected_order['order_id_'], selected_order['price'], 0, {"display": "block"}
            return None, None, None, {"display": "none"}

        @self.app.callback(
            Output("liquibook-order-cancel-modify", "data"),
            Input("modify-order-button", "n_clicks"),
            Input("cancel-order-button", "n_clicks"),
            State("new-price-input", "value"),
            State("delta-quantity-input", "value"),
            State("selected-order", "data"),
            prevent_initial_call=True
        )
        def handle_order(modify_clicks, cancel_clicks, new_price, delta_quantity, selected_order):
            triggered = callback_context.triggered_id
            if triggered == "cancel-order-button":
                return self.liquibook_adapter.cancel_order(selected_order)
            else:
                return self.liquibook_adapter.modify_order(selected_order, delta_quantity, new_price)