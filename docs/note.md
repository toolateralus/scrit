
Please, if you'd like to contribute to this documentation making it more robust, feel free to make pull requests with any amount of change.
Thanks.

This is currently linux-only but most of that is just for `dlopen` for modules & the breakpoint debugger using some unix functions.
Feel free to modify this and make it cross-platform compatible: I just have no interest in doing so.

Update: I have removed a lot of the linux only code, it's just the module loading is linux only (due to a hard coded /usr path)
And the install script / installation process is linux only.