import dash
import dash_ag_grid as dag
from dash import dcc, Input, Output, html, callback_context


class PriceDepthGrid(object):
    def __init__(self, app: dash.Dash):
        self.app = app

        self.grid = dag.AgGrid(
            id="price-depth-grid",
            columnDefs=[
                {
                    "headerName": "BidPrice",
                    "field": "bid_price",
                    "valueFormatter": {
                        "function": "params.data && params.data.bid_size !== 0 ? params.value : ''"
                    },
                },
                {
                    "headerName": "AskPrice",
                    "field": "ask_price",
                    "valueFormatter": {
                        "function": "params.data && params.data.ask_size !== 0 ? params.value : ''"
                    },
                },
                {
                    "headerName": "BidSize",
                    "field": "bid_size",
                    "valueFormatter": {
                        "function": "params.value !== 0 ? params.value : ''"
                    },
                },
                {
                    "headerName": "AskSize",
                    "field": "ask_size",
                    "valueFormatter": {
                        "function": "params.value !== 0 ? params.value : ''"
                    },
                },
            ],
            rowData=[],
            dashGridOptions={"rowSelection": "single"},
            defaultColDef={"flex": 1, "minWidth": 100, "resizable": False},
            style={"height": "290px", "width": "100%"},
            className="ag-theme-quartz-dark",
        )

        self._register_callbacks()

    def layout(self):
        return html.Div([
            dcc.Store(id="liquibook-state-change"),
            dcc.Store(id="liquibook-order-cancel-modify"),
            self.grid,
        ])

    def _register_callbacks(self):
        @self.app.callback(
            Output("price-depth-grid", "rowData"),
            Input("liquibook-state-change", "data"),
            Input("liquibook-order-cancel-modify", "data"),
            prevent_initial_call=True
        )
        def on_price_depth_change(price_depth_change, price_depth_change_modify):

            triggered = callback_context.triggered_id

            if triggered == "liquibook-state-change":
                return price_depth_change['price_depth']
            elif triggered == "liquibook-order-cancel-modify":
                return price_depth_change_modify['price_depth']

            return None

        @self.app.callback(
            Output("selected-price-level", "data"),
            Input("price-depth-grid", "cellClicked")
        )
        def display_selected(selected):
            if not selected:
                return None

            col_id = selected.get("colId")
            if col_id in {"bid_price", "ask_price"}:
                return selected.get("value")

            return None
