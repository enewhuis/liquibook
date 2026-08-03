# Copyright (c) Mike Kipnis
import liquibook

import dash
import dash_bootstrap_components as dbc
from dash import html

from components import Orders
from components.PriceDepth import PriceDepthGrid
from components.OrderEntry import OrderFormComponent
from components.OrderCancelModify import OrderCencelModifyComponent
from liquibook_adapter.LiquiBookAdapter import LiquiBookAdapter
import logging

logging.basicConfig(
    level=logging.DEBUG,  # Set the minimum level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",  # Log format
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[
        logging.StreamHandler(),               # Print to console
        logging.FileHandler("liquibook_adapter.log", "a"),   # Save logs to a file
    ]
)

class LiquiBook_Sandbox(object):

    def __init__(self, app: dash.Dash, adapter: LiquiBookAdapter, market_instrument: dict):
        self.app = app
        self.liquibook_adapter = adapter
        self.market_instrument = market_instrument
        self.order_form_component = OrderFormComponent(self.app,
                                                       self.liquibook_adapter,
                                                       self.market_instrument)

        self.order_cancel_modifu = OrderCencelModifyComponent(self.app, self.liquibook_adapter)

        self.trade_order_history = Orders.TradeOrderHistory(self.app)

    def layout(self):

        return dbc.Container([
            dbc.Navbar(
                dbc.NavbarBrand("LiquiBook Sandbox", href="#",
                                style={"margin-left": "20px", "fontSize": "20px"}), color="Info", dark=True
            ),
            dbc.Row(
                dbc.Col(
                    html.Hr(
                        style={
                            "borderTop": "2px solid #6c757d",
                            "margin": "0px 10px 20px"
                        }
                    ),
                    width=12
                )
            ),
            dbc.Row([
                dbc.Col(PriceDepthGrid(self.app).layout()),
                dbc.Col([
                    dbc.Row([
                        self.order_form_component.layout(),
                    ]),
                    dbc.Row([
                        self.order_cancel_modifu.layout(),
                    ])
                ])

            ], className="p-3"),
            dbc.Row([self.trade_order_history.layout()], className="p-3"),

        ], fluid=True, className="p-0")

    def get_order_form_component(self):
        return self.order_form_component


if __name__ == '__main__':

    app = dash.Dash(__name__, suppress_callback_exceptions=True, title="LiquiBook Sandbox",
                    external_stylesheets=[dbc.themes.SUPERHERO])

    sandbox_instrument = {'symbol':'SANDBOX','market_price': 10095}
    logger = logging.getLogger("LiquiBookAdapter")

    liquibook_adapter = LiquiBookAdapter(sandbox_instrument)
    sandbox = LiquiBook_Sandbox(app, liquibook_adapter, sandbox_instrument)
    app.layout = sandbox.layout()

    app.run(host="0.0.0.0", port=8050, debug=True)
