def convert( inputFileName = "./templates/index.html", outputFileName = "./esp.txt" ):
    outputFile = open( outputFileName, "w+" )

    htmlFile = open( inputFileName, "r" )

    lines = htmlFile.readlines()

    outputFile.write( 'String ptr = "' + lines[ 0 ].replace( "\n", "" ).replace( '"', "'" ) + '\\n";\n' )

    for line in lines[ 1: ]:
        outputFile.write( 'ptr += "' + line.replace( "\n", "" ).replace( '"', "'" ) + '\\n";\n' )
    outputFile.close()

if __name__ == "__main__":
    convert()