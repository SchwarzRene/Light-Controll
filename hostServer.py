import flask


app = flask.Flask( __name__ )

@app.route( "/" )
def index():
    return flask.render_template( "index.html" )

if __name__ == "__main__":
    app.run( host = "192.168.1.94", port =  5000 )