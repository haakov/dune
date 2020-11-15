defmodule Elixir_srv.Server do
    use Plug.Router

    plug Plug.Static, at: "/", from: :example

    plug :match
    plug :dispatch

    get "/" do
      conn = put_resp_content_type(conn, "text/html")
      send_file(conn, 200, "../index.html")
    end

    match _ do
      cond do
        File.exists?(".." <> conn.request_path) ->
          send_file(conn, 200, ".." <> conn.request_path)

        List.first(conn.path_info) == "delete" ->
          String.slice(conn.request_path, 7..-1) |>
          File.rm_rf
          send_resp(conn, 200, "done")

        true ->
          send_resp(conn, 404, "sorry")
      end
    end
end
