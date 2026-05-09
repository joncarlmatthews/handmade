# Day 020 - Debugging the Audio Sync

I think the solution was overly complex. I've chosen to write the frame's audio at the earliest possible place (which is at the write cursor) I then opt to write at least 4 frames worth of audio, to avoid gaps in the audio if the framerate were to unexpectedly drop. That's it. That's my solution.
