import pandas as pd
import numpy as np
import sys

import warnings
warnings.filterwarnings('ignore', message='.*converting a masked element to nan.*')

alias = {
    "sophia": "sophia (rust)",
    "librdf": "librdf (c)",
    "jena": "jena (java)",
    "n3js": "n3js (js)",
    "python": "rdflib (python)",
}
color_key = {
    "sophia (rust)": "red",
    "sophia_lg": "darkorange",
    "librdf (c)": "purple",
    "jena (java)": "black",
    "n3js (js)": "blue",
    "sophia_wasm": "darkorange",
    "sophia_wasm_lg": "red",
    "python": "green",
    "pypy": "darkgreen",
    
    "sophia-v0.1.0": "#888888",
    "sophia-v0.2.0": "#996666",
    "sophia-v0.3.0": "#BB3333",
    "sophia-v0.4.0": "#EE0000",
}

def load_data(task, *tools):
    dfs = []
    for tool in tools:
        try:
            df = pd.read_csv("csv/{}-{}.csv".format(task, tool))
            df['tool'] = alias.get(tool, tool)
            dfs.append(df)
        except FileNotFoundError as ex:
            print(ex, file=sys.stderr)
    df = pd.concat(dfs)
    df.index = range(len(df.index))
    
    if task == 'query':
        df['t_query'] = (df.t_first + df.t_rest)
        df['r_load'] = (df['size'] / df.t_load)
    elif task == 'parse':
        df['r_parse'] = (df['size'] / df.t_parse)
    return df.groupby(['tool', 'size'])

def my_plot(data, attr_name, *, exclude=[], savename=None, **kw):
    means = data[attr_name].mean().unstack().transpose()
    stdev = data[attr_name].std().unstack().transpose()
    for i in exclude:
        try:
            del means[i]
            del stdev[i]
        except:
            pass
    color = list(means.columns.map(color_key.get))
    ax = means.plot(yerr=2*stdev, grid=1, color=color, **kw)
    if savename:
        ax.get_figure().savefig("figures/{}.svg".format(savename))
    return ax

def plot_query_stats(data):
    my_plot(data, "t_load", title="Time (in s) to load an NT file in memory", loglog=True)
    #my_plot(data, "t_load", xlim=(0,200_000), ylim=(0,10), savename="t_load_lin", title="Time (in s) to load an NT file in memory")

    my_plot(data, "r_load", title="Load rate (in triple/s) from an NT file in memory", logx=True)

    my_plot(data, 'm_graph', title="Memory (in kB) used while allocating for the graph", loglog=True, exclude=['jena'])

    my_plot(data, 't_first', title="Time (in s) to retrieve the first matching triple (*,p,o)", loglog=True)
    
    my_plot(data, 't_query', title="Time (in s) to retrieve all matching triples (*,p,o)", loglog=True)
    #my_plot(data, 't_query', xlim=(0,1_000_000), ylim=(0, 0.1), title="Time (in s) to retrieve all matching triples (*,p,o)", savename="t_query_lin")

  