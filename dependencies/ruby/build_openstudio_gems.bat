REM first argument is location to install gems
set PATH=%PATH%;C:\Program Files\Git\cmd
rmdir /S /Q "%1"
bundle install --path="%1"
rmdir /S /Q "%1/ruby/2.0.0/cache"