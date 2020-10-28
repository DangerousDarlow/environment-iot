import plotly.graph_objects as go
import pandas as pa

data = pa.read_csv('data/am2320 consistency data.csv')

fig = go.Figure()

for series in sorted(filter(lambda s: 'temperature' in s, data)):
    fig.add_trace(go.Histogram(
        x=data[series],
        name=series[len('temperature '):],
        histnorm='percent',
        xbins=dict(
            start=20,
            end=25,
            size=0.1
        ),
        opacity=0.75
    ))

fig.update_layout(
    title_text='AM2320 Temperature Measurements',
    xaxis_title_text='Temperature',
    yaxis_title_text='Percentage',
    bargap=0.2,
    bargroupgap=0.1
)

fig.show()
