defmodule Elixir_srv.Server do
    use Plug.Router

    plug Plug.Static, at: "/", from: :example

    plug :match
    plug :dispatch

    get "/" do
      conn = put_resp_content_type(conn, "text/html")
      send_file(conn, 200, "/home/hkon/ttk22/dune/source/www/index.html")
    end

    match _ do
      cond do
        File.exists?("/home/hkon/ttk22/dune/source/www" <> conn.request_path) ->
          send_file(conn, 200, "/home/hkon/ttk22/dune/source/www" <> conn.request_path)

        List.first(conn.path_info) == "dune" ->
          send_file(conn, 200, "/home/hkon/ttk22/dune/source/www/state/" <> List.last(conn.path_info))

        List.first(conn.path_info) == "delete" ->
          String.slice(conn.request_path, 7..-1) |>
          File.rm_rf
          send_resp(conn, 200, "done")

          #File.rmdir()

        true ->
          send_resp(conn, 404, "sorry")
      end
    end
end
