import dash_ag_grid as dag
from dash import html, dash, Output, Input, dcc, callback_context

# Dummy price formatter JS function string (replace with actual if needed)
price_formatter = {
    "function": "params.value !== 0 ? params.value : ''"
}

side_formatter = {
    "function": "params.value === true ? 'Buy' : 'Sell'"
}


class TradeOrderHistory(object):

    def __init__(self, app: dash.Dash):
        self.app = app
        self.row_data = []

        self._register_callbacks()

    def layout(self):

        column_defs = [
            {"headerName": "OrderId", "field": "order_id_", "sortable": True, "flex": 2, "filter": "agTextColumnFilter"},
            #{"headerName": "state", "field": "state", "sortable": True, "flex": 2},
            {"headerName": "OrderState", "field": "state_str", "sortable": True, "flex": 2},
            {"headerName": "Side", "field": "is_buy", "sortable": True, "flex": 2, "valueFormatter": side_formatter, "cellRenderer": "agTextCellRenderer"},
            {"headerName": "Price", "field": "price", "sortable": True, "flex": 2, "valueFormatter": price_formatter},
            {"headerName": "StopPx", "field": "stop_price", "sortable": True, "flex": 2, "valueFormatter": price_formatter},
            {"headerName": "Quantity", "field": "order_qty", "sortable": True, "flex": 2},
            {"headerName": "OpenQty", "field": "open_qty", "sortable": True, "flex": 2},
            {"headerName": "FilledQty", "field": "filled_qty", "sortable": True, "flex": 2, "valueFormatter": price_formatter},
            #{"headerName": "All or None", "field": "all_or_none", "sortable": True, "flex": 2},
            #{"headerName": "Immediate or Cancel", "field": "immediate_or_cancel", "sortable": True, "flex": 2},
            #{"headerName": "conditions", "field": "conditions", "sortable": True, "flex": 2},
            {"headerName": "Condition", "field": "conditions_str", "sortable": True, "flex": 2},
        ]

        return html.Div([
            dcc.Store(id="order-book-state", data={}),
            dag.AgGrid(
                id="order-grid",
                columnDefs=column_defs,
                rowData=self.row_data,
                dashGridOptions={"rowSelection": "single"},
                defaultColDef={"flex": 1, "minWidth": 100, "resizable": True},
                style={"height": "400px", "width": "100%"},
                className="ag-theme-quartz-dark"
            )
        ])

    def _register_callbacks(self):
        @self.app.callback(
            Output("order-grid", "rowData"),
            Input("liquibook-state-change", "data"),
            Input("liquibook-order-cancel-modify", "data"),
            prevent_initial_call=True
        )
        def order_book_state(order_state, order_state_cancel_modify):
            triggered = callback_context.triggered_id
            if triggered == "liquibook-state-change":
                return order_state['orders']
            elif triggered == "liquibook-order-cancel-modify":
                return order_state_cancel_modify['orders']
            return None

        @self.app.callback(
            Output("selected-order", "data"),
            Input("order-grid", "selectedRows")
        )
        def display_selected(selected):
            if selected:
                return selected[0]
            return None