<?php

namespace App\Http\Controllers;

use App\Models\Channel;
use App\Models\Stream;
use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Str;

class ChannelController extends Controller
{
    public function search(Request $request) {
        $query = $request->input('query');
        $users = User::where('username', 'like', "%{$query}%")->get();
        foreach ($users as $user) {
            $user->profile_picture = $user->profile_picture
                ? asset('storage/' . $user->profile_picture)
                : null;
        }
        return response()->json(["users" => $users]);

    }

    public function show($userId)
    {
        $user = User::findOrFail($userId);
        $channel = $user->channel;

        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }

        return response()->json([
            "channel" => $channel,
            "user" => $user
        ]);
    }

    public function update(Request $request)
    {
        $user = Auth::user();
        $channel = $user->channel;

        $data = $request->validate([
            'title' => 'nullable|string|max:255',
            'description' => 'nullable|string',
            'is_live' => 'nullable|boolean',
            'stream_title' => 'nullable|string|max:255',
            'stream_description' => 'nullable|string',
            'stream_category' => 'nullable|string|max:255',
        ]);

        $channel->update($data);

        return response()->json(["channel" => $channel]);
    }
    public function start(Request $request)
    {
        $request->validate([
            'stream_key' => 'required|string',
        ]);

        $channel = Channel::where('stream_key', $request->stream_key)->firstOrFail();

        // End any previous live streams
        Stream::where('channel_id', $channel->id)->where('is_live', true)
            ->update(['is_live' => false, 'end_time' => now()]);

        $stream = Stream::create([
            'channel_id' => $channel->id,
            'title' => $request->input('title', null),
            'start_time' => now(),
            'is_live' => true,
        ]);

        // Optionally set channel as live
        $channel->is_live = true;
        $channel->save();

        return response()->json(['stream' => $stream]);
    }

    // End a stream (called when streaming ends)
    public function end(Request $request)
    {
        $request->validate([
            'stream_key' => 'required|string',
        ]);

        $channel = Channel::where('stream_key', $request->stream_key)->firstOrFail();

        $stream = Stream::where('channel_id', $channel->id)
            ->where('is_live', true)
            ->latest('start_time')
            ->first();

        if ($stream) {
            $stream->update([
                'end_time' => now(),
                'is_live' => false,
            ]);
        }

        // Optionally set channel as offline
        $channel->is_live = false;
        $channel->save();

        return response()->json(['message' => 'Stream ended']);
    }
    public function regenerateKey($id)
    {
        $channel = Channel::where('id', $id)->firstOrFail();
        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $user = Auth::user();
        if( $user->id != $channel->user_id) {
            return response()->json(["error" => "Unauthorized user."], 403);
        }

        // Generate a new unique key
        do {
            $newKey = Str::random(32);
        } while (Channel::where('stream_key', $newKey)->exists());

        $channel->stream_key = $newKey;
        $channel->save();

        return response()->json(['channel' => $channel]);
    }
    public function settings($id)
    {
        $channel = Channel::where('id', $id)->firstOrFail();
        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $user = Auth::user();
        if( $user->id != $channel->user_id) {
            return response()->json(["error" => "Unauthorized user."], 403);
        }
        $host = request()->getHost();
        return response()->json(["channel" => $channel, "host" => $host]);
    }
    public function stream($id)
    {
        $channel = Channel::where('id', $id)->firstOrFail();
        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $host = request()->getHost();
        return response()->json(["channel" => $channel, "host" => $host]);
    }
}
