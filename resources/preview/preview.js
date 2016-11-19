/**
 * Logic invoked from C++ code
 */ 
class RstPadApi {
    /**
     * @param {Number} position
     */         
    static scrollToPosition(position) {
        window.scrollTo(0, position);
    }
    
    /**
     * @param {Number} line
     */ 
    static scrollToLine(line) {
        let position = this.getPositionForLine(line);
        
        if (null !== position) {
            this.scrollToPosition(position);
        }
    }
    
    /**
     * @param {Number} line
     * @returns {Number|null}     
     */ 
    static getPositionForLine(line) {
        let map = this.getLineMap();
        
        //console.log('getPositionForLine ' + line);
    
        // try to use element at the exact line
        if (map[line]) {
            //console.log('found exact element for line ' + line);
            return this.getElementPosition(map[line]);
        }
    
        // find the nearest lines
        let nearestPrevLine = null;
        let nearestNextLine = null;
        
        for (let mappedLine in map) {
            if (map.hasOwnProperty(mappedLine)) {
                mappedLine = mappedLine / 1;
                //console.log('mappedLine = ' + mappedLine + '(' + typeof(mappedLine) + ') nearestPrevLine = ' + nearestPrevLine + ' nearestNextLine = ' + nearestNextLine);
                
                if (mappedLine < line) {
                    if (null === nearestPrevLine || mappedLine > nearestPrevLine) {
                        nearestPrevLine = mappedLine;
                    }
                } else if (null === nearestNextLine || mappedLine < nearestNextLine) {
                    nearestNextLine = mappedLine;
                }
            }
        }
        
        //console.log('nearestPrevLine = ' + nearestPrevLine + ' nearestNextLine = ' + nearestNextLine);
        
        // estimate a position between the previous and next lines
        if (nearestPrevLine && nearestNextLine) {
            let prevElemPos = this.getElementPosition(map[nearestPrevLine], 'bottom');
            let nextElemPos = this.getElementPosition(map[nearestNextLine], 'top');
            
            //console.log('prevElemPos = ' + prevElemPos + ' nextElemPos = ' + nextElemPos);
            //console.log('percentage = ' + ((line - nearestPrevLine) / (nearestNextLine - nearestPrevLine)));
            
            if (prevElemPos < nextElemPos) {
                return prevElemPos + (nextElemPos - prevElemPos) * (line - nearestPrevLine) / (nearestNextLine - nearestPrevLine);
            }
        }
        
        // only one of the lines was found or the positions are unexpected
        if (nearestPrevLine || nearestNextLine) {
            //console.log('using one of the lines = ' + (nearestPrevLine || nearestNextLine));
            return this.getElementPosition(map[nearestPrevLine || nearestNextLine]);
        }
        
        // failure
        //console.log('failure');
        return null;
    }
    
    /**
     * @returns {Object} line => HTMLElement
     */         
    static getLineMap() {
        if (!this._lineMap) {
            this._lineMap = {}
            let elements = document.querySelectorAll('*[data-line]');
            
            for (let i = 0; i < elements.length; ++i) {
                this._lineMap[elements[i].dataset.line] = elements[i];
            }
        }
        
        return this._lineMap;
    }
    
    /**
     * @param {HTMLElement} elem
     * @param {String}      type     
     * @returns {Number}
     */         
    static getElementPosition(elem, type = 'top') {
        return window.pageYOffset + elem.getBoundingClientRect()[type];
    }
}

