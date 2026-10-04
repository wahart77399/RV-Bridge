require 'webrick'
require 'json'
require 'open3'

portal = File.expand_path('../../data/SmartCoachDevicePortal', __dir__)
images = File.expand_path('../../images', __dir__)
port = Integer(ENV.fetch('SMARTCOACH_PREVIEW_PORT', '8091'))
input, output, process = Open3.popen2(ARGV.fetch(0), '--preview')
lock = Mutex.new
server = WEBrick::HTTPServer.new(
  Port: port, BindAddress: '127.0.0.1', DocumentRoot: portal,
  AccessLog: [], Logger: WEBrick::Log.new($stderr, WEBrick::Log::WARN)
)
server.mount('/images', WEBrick::HTTPServlet::FileHandler, images)
['/devices.json', '/coach.json', '/discovery', '/status', '/devices/review', '/devices/rename', '/coach', '/reboot'].each do |path|
  server.mount_proc(path) do |request, response|
    lock.synchronize do
      input.puts(JSON.generate(path: path, method: request.request_method, body: request.body || ''))
      input.flush
      result = JSON.parse(output.gets)
      response.status = result.fetch('code')
      response['Content-Type'] = 'application/json'
      response.body = result.fetch('body')
    end
  end
end
trap('INT') { server.shutdown }
trap('TERM') { server.shutdown }
puts "Mock portal: http://127.0.0.1:#{port} (no coach connection)"
$stdout.flush
begin
  server.start
ensure
  input.close
  output.close
  process.value
end