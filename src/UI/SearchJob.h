#ifndef RSTPAD_SEARCHJOB_H
#define RSTPAD_SEARCHJOB_H

#include <QString>

namespace RstPad {

    class SearchJob
    {
        public:
            enum Mode {
                Find,
                Replace,
                ReplaceAll,
                Count,
            };

            SearchJob();
            Mode mode = Find;
            QString text = QString();
            QString replacement = QString();
            bool backwards = false;
            bool caseSensitive = false;
            bool wholeWords = false;
            bool wildcards = false;
    };

}

#endif // RSTPAD_SEARCHJOB_H
