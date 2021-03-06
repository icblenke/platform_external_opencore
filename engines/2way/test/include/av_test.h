/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef AV_TEST_H_INCLUDED
#define AV_TEST_H_INCLUDED

#include "test_base.h"


class av_test : public test_base
{
    public:
        av_test(bool aUseProxy = false)
                : test_base(aUseProxy, 1)
        {
            iUsingAudio = true;
            iUsingVideo = true;
            inumCalled = 0;
            iTestName = _STRLIT_CHAR("av");
        }

        ~av_test()
        {
        }

        void test();

        void Run();

        void DoCancel();


        virtual void TimerCallback();
        void FinishTimerCallback();
    protected:
        bool start_async_test();
        bool CheckAllSourceAndSinkAdded();

        int inumCalled;

    private:
        virtual void ConnectSucceeded();
        virtual void InitFailed();
        virtual void ConnectFailed();
        virtual void AudioAddSinkSucceeded();
        virtual void AudioAddSourceSucceeded();
        virtual void VideoAddSinkSucceeded();
        virtual void VideoAddSourceSucceeded();
        virtual void RstCmdCompleted();

        virtual void AudioRemoveSourceCompleted();
        virtual void AudioRemoveSinkCompleted();

        virtual void VideoRemoveSourceCompleted();
        virtual void VideoRemoveSinkCompleted();

        void CheckForTimeToDisconnect();
};


#endif


